package com.github.jodersky.ace

import com.github.jodersky.ace._
import com.github.jodersky.ace.jssc._
import scala.concurrent.ExecutionContext.Implicits.global
import _root_.jssc.SerialPort


object Main {
  
  
  
  def main(args: Array[String]): Unit = {
    val serial = new Serial("/dev/ttyACM0")
    val framer = new Framer
    val arq = new Arq(200)
    val app = new SimpleActionLayer((s: Seq[Int]) => println(s))
    
    serial connect framer connect arq connect app
    serial.begin()
    
    while (true) {
      app.send(Console.readLine.getBytes().map(_.toInt)).map(sent => Console.println("> " + sent))
      
    }
    
  }
  

}