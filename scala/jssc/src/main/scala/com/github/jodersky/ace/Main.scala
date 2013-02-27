package com.github.jodersky.ace

import com.github.jodersky.ace.protocol._

import scala.concurrent.ExecutionContext.Implicits.global

object Main {
  
  def main(args: Array[String]): Unit = {
    import com.github.jodersky.ace.jssc._
    
    val s = serial.Serial.open("/dev/ttyACM0", 9600)
    val framer = new Framer
    val arq = new Arq(200)
    val app = new SimpleActionLayer((s: Seq[Int]) => println(s))
    
    s connect framer connect arq connect app
    s.begin()
    
    while (true) {
      app.send(Console.readLine.getBytes().map(_.toInt)).map(sent => Console.println("> " + sent))
      
    }
      
    
    ()
  }
  

}