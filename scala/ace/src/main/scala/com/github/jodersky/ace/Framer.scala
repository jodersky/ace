package com.github.jodersky.ace

import scala.collection.mutable.ArrayBuffer
import scala.concurrent.Future
import scala.concurrent.ExecutionContext.Implicits.global

/** A framer takes bytes (as unsigned integers) and creates frames.
 *  Note that the input type of this reactive layer is also a sequence of
 *  integers for performance reasons (i.e. a future will not be created for every byte sent).
 */
class Framer extends ReactiveLayer[Seq[Int], Seq[Int]] {
  import Framer._

  private var state: State = Waiting
  private val buffer = new ArrayBuffer[Int]

  protected def receive(bytes: Seq[Int]) = bytes foreach receive
  
  protected def receive(byte: Int): Unit = {

    state match {
      case Escaping => {
        buffer += byte
        state = Receiving
      }
      case Waiting => if (byte == Start) {
        buffer.clear()
        state = Receiving
      }
      case Receiving => byte match {
        case Escape => state = Escaping
        case Start => buffer.clear()
        case Stop => {
          state = Waiting
          if (checksum(buffer.init) == buffer.last)
            notifyHigher(buffer.init)
        }
        case datum => buffer += datum
      }
    }
  }

  def send(data: Seq[Int]): Future[Seq[Int]] = {
    val buffer = new ArrayBuffer[Int]
    
    buffer += Start    
    data foreach { byte =>
      byte match {
        case Start | Stop | Escape => {
          buffer += Escape
          buffer += byte
        }
        case _ => buffer += byte
      }
    }
    val c = checksum(data)
    c match {
        case Start | Stop | Escape => {
          buffer += Escape
          buffer += c
        }
        case _ => buffer += c
      }
    buffer += Stop
    sendToLower(buffer) map (_ => data)
  }
}

object Framer {
  sealed trait State
  case object Waiting extends State
  case object Receiving extends State
  case object Escaping extends State

  final val Escape = 0x10
  final val Start = 0x02
  final val Stop = 0x03

  def checksum(unsignedData: Seq[Int]) = {
    unsignedData.fold(0)(_ ^ _)
  }

}