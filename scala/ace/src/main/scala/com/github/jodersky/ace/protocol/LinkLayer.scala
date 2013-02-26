package com.github.jodersky.ace.protocol

import scala.collection.mutable.ArrayBuffer
import scala.concurrent.Future
import scala.concurrent.ExecutionContext.Implicits.global

class LinkLayer extends ReactiveLayer[Array[Byte], Packet] {
  import LinkLayer._

  private var state: State = Waiting
  private val buffer = new ArrayBuffer[Int]

  def receive(data: Array[Byte]) = for (d <- data) receive(d)

  def receive(data: Byte): Unit = {
    val unsigned = data & 0xff

    state match {
      case Escaping => {
        buffer += unsigned
        state = Receiving
      }
      case Waiting => if (unsigned == Start) {
        buffer.clear()
        state = Receiving
      }
      case Receiving => unsigned match {
        case Escape => state = Escaping
        case Start => buffer.clear()
        case End => {
          state = Waiting
          if (checksum(buffer.init.toArray) == buffer.last)
            notifyHigher(Packet(buffer.init.toArray))
        }
        case datum => buffer += datum
      }
    }
  }

  def write(packet: Packet): Future[Packet] = {
    val buffer = new ArrayBuffer[Int]
    buffer += Start
    packet.data foreach { unsigned =>
      unsigned match {
        case Start | End | Escape => {
          buffer += Escape
          buffer += unsigned
        }
        case _ => buffer += unsigned
      }
    }
    buffer += checksum(packet.data)
    buffer += End
    writeToLower(buffer.map(_.toByte).toArray).map(_ => packet)
  }
}

object LinkLayer {
  sealed trait State
  case object Waiting extends State
  case object Receiving extends State
  case object Escaping extends State

  final val Escape = 0x02
  final val Start = 0x03
  final val End = 0x10

  def checksum(unsignedData: Seq[Int]) = {
    unsignedData.fold(0)(_ ^ _)
  }

}