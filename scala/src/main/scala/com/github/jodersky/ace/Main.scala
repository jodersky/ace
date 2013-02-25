package com.github.jodersky.ace

import scala.concurrent._
import scala.concurrent.ExecutionContext.Implicits.global

object Main {
  
  def main(args: Array[String]): Unit = { 
    val s = SafeSerial.open("/dev/ttyACM0", 115200).get
    var cmd: String = ""
    while (cmd != ":quit"){
      cmd = Console.readLine
      s.send(cmd).onComplete(v => println("sent: " + v))
    }
    s.close()
  }

}
