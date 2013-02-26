package com.github.jodersky.ace

import scala.concurrent._
import scala.concurrent.ExecutionContext.Implicits.global
import scala.concurrent.duration._
import java.io.IOException
import scala.collection.mutable.HashMap
import scala.collection.mutable.Queue
import scala.util.Success

class Arq(timeout: Int, maxResends: Int = 5, maxMessageBuffer: Int = 10) extends ReactiveLayer[Seq[Int], Seq[Int]] {
  import Arq._

  case class OpenMessage(data: Seq[Int], promise: Promise[Seq[Int]])

  // a map containing yet to be acknowledged messages
  private val openMessages = HashMap[Int, OpenMessage]()

  // received message sequences
  private val receivedSequences = Queue[Int]()

  def receive(frameData: Seq[Int]) = {
    val sequence = frameData(SequenceOffset)
    val command = frameData(CommandOffset)
    val message = frameData.drop(MessageOffset)

    command match {
      
      case Ack => {
        openMessages.get(sequence) map {
          case OpenMessage(data, promise) => promise.complete(Success(data))
        }
      }

      case Data => {
        writeToLower(ack(sequence))

        if (!(receivedSequences contains sequence)) {
          if (receivedSequences.size > maxMessageBuffer) receivedSequences.dequeue
          receivedSequences enqueue sequence
          notifyHigher(message)
        }
      }

    }
  }

  def write(message: Seq[Int]) = {
    val promise = Promise[Seq[Int]]
    val sequence = nextSequence()
    
    val frameData = Seq(sequence, Data) ++ message 

    def send(n: Int): Future[Seq[Int]] =
      writeToLower(frameData) map { frameData =>
        Await.result(promise.future, timeout.milliseconds)
      } recoverWith {
        case t: TimeoutException if (n < maxResends) => send(n + 1)
      }

    if (openMessages.size >= maxMessageBuffer) Future.failed(new IOException("too many open requests"))
    else {
      openMessages += (sequence -> OpenMessage(message, promise))
      send(0) andThen { case successOrFailure => (openMessages -= sequence) }
    }
  }

}

object Arq {
  final val MaxSequence = 255
  final val Data = 0x05
  final val Ack = 0x06

  final val SequenceOffset = 0
  final val CommandOffset = 1
  final val MessageOffset = 2

  private[this] var seq = 0
  private def nextSequence() = { seq += 1; if (seq > MaxSequence) seq = 0; seq }

  def ack(seq: Int) = Seq(seq, Ack)
}