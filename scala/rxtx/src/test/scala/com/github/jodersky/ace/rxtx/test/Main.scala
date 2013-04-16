package com.github.jodersky.ace.rxtx.test

import com.github.jodersky.ace.Arq;
import com.github.jodersky.ace.Framer;
import com.github.jodersky.ace._
import com.github.jodersky.ace.rxtx._
import scala.concurrent.ExecutionContext.Implicits.global


object Main {
  
  def main(args: Array[String]): Unit = {
    val serial = new Serial("/dev/ttyACM0", 9600)
    val framer = new Framer
    val arq = new Arq(200)
    val app = new SimpleActionLayer((s: Seq[Int]) => println(s))
    
    serial connect framer connect arq connect app
    serial.begin()
    
    print(">")
    while (true) {
      val bytes = readLine.getBytes().map(_.toInt)
      app.send(bytes).map(sent => "[sent] " + sent).recover{case ex => "[failed] " + ex}.map{s => println(s); print(">")}
    }
    
  }
  
}
