package com.github.jodersky.ace.protocol

import scala.concurrent._
import scala.concurrent.ExecutionContext.Implicits.global
import scala.concurrent.duration._
import java.io.IOException
import scala.collection.mutable.HashMap
import scala.collection.mutable.Queue
import com.github.jodersky.ace.protocol.{Packet => PPacket}
import scala.util.Success

class TransportLayer extends ReactiveLayer[Packet, Message] {
  
  
  private val openRequests = HashMap[Int, (Message, Promise[Message])]()
  private val receivedSeqs = Queue[Int]()
  
  class Packet(val data: Seq[Int]) {
    
    def seq = data(0)
    def cmd = data(1)
    def message = Message(data.drop(2))
    
    def underlying = PPacket(data)
  }
  
  object Packet {
    final val DATA = 0x05
    final val ACK = 0x06
    private var seq = 0;
    private def nextSeq() = {seq += 1; if (seq > TransportLayer.MaxSeq) seq = 0; seq}
    
    def fromPacket(packet: PPacket) = new Packet(packet.data)
    
    def fromMessage(message: Message) = new Packet(Seq(nextSeq(), DATA) ++ message.data)
    
    def ack(seq: Int) = new Packet (Seq(seq, ACK))
    
  }   

  def receive(ppacket: PPacket) = {
    val in = Packet.fromPacket(ppacket)
    
    in.cmd match {
      case Packet.ACK => {
        openRequests.get(in.seq).map{case (message, promise) => promise.complete(Success(message))}
      }
      
      case Packet.DATA => {
         writeToLower(Packet.ack(in.seq).underlying)
         
         if (!(receivedSeqs contains in.seq)) {
           if (receivedSeqs.size > TransportLayer.MaxSeqBuffer) receivedSeqs.dequeue
           receivedSeqs enqueue in.seq
           notifyHigher(in.message)
         }  
      }
      
    }
  }

  def write(message: Message) = {
    val promise = Promise[Message]
    val packet = Packet.fromMessage(message)
    val seq = packet.seq

    def send(n: Int): Future[Message] =
      writeToLower(packet.underlying) map { packet =>
        Await.result(promise.future, TransportLayer.Timeout milliseconds)
      } recoverWith {
        case t: TimeoutException if (n < TransportLayer.MaxResends) => send(n + 1)
      }
      
    if (openRequests.size >= TransportLayer.MaxSeqBuffer) Future.failed(new IOException("too many open requests"))
    else {
      openRequests += (seq -> (message, promise))
      send(0) andThen {case r => (openRequests -= seq)}
    }
  }

}

object TransportLayer {
  final val Timeout = 100
  final val MaxResends = 5
  final val MaxSeq = 255
  final val MaxSeqBuffer = 10
}