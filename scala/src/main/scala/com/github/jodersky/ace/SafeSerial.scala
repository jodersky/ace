package com.github.jodersky.ace

import com.github.jodersky.ace.protocol._
import jssc.SerialPort
import scala.concurrent.ExecutionContext.Implicits.global
import scala.util.Try

class SafeSerial(port: SerialPort) { self =>
  val physical = new PhysicalLayer(port)
  val link = new LinkLayer
  val transport = new TransportLayer

  val application = new ReactiveLayer[Message, String] {
    def receive(message: Message) = Console.println(message.data.map(_.toChar).mkString(""))
    def write(s: String) = writeToLower(Message(s.map(_.toChar.toInt))).map(x => s)
  }

  def send(s: String) = application.write(s)
  def close() = Try(port.closePort())

  physical connect link connect transport connect application
  physical.begin()
}

object SafeSerial {
  def open(port: String, rate: Int) = Try {
    val serialPort = new SerialPort(port);
    serialPort.openPort()
    serialPort.setParams(rate,
      SerialPort.DATABITS_8,
      SerialPort.STOPBITS_1,
      SerialPort.PARITY_NONE)
    serialPort
  } map (port => new SafeSerial(port)) 
}