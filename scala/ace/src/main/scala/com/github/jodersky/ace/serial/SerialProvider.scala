package com.github.jodersky.ace.serial

trait SerialProvider {
  
  def open(port: String, baudRate: Int): Serial

}