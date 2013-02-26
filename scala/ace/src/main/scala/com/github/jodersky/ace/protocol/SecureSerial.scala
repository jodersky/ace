package com.github.jodersky.ace.protocol

import jssc.SerialPort
import scala.concurrent.ExecutionContext.Implicits.global
import scala.util.Try

class SecureSerial(port: SerialPort) { self =>
  val physical = new PhysicalLayer(port)
  val link = new LinkLayer
  val transport = new TransportLayer
  
  val application = new ReactiveLayer[Message, String] {
    def receive(message: Message) = Console.println(message.data.mkString(""))
    def write(s: String) = writeToLower(Message(s.map(_.toInt))).map(x => s)
  }
  
  def send(s: String) = application.write(s)
  def close() = Try(port.closePort())
  
  physical connect link connect transport connect application
  physical.begin()
}