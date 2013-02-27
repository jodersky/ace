package com.github.jodersky.ace.serial

import com.github.jodersky.ace.protocol.ReactiveLayer

trait Serial extends ReactiveLayer[Nothing, Seq[Int]] {
  protected def receive(nothing: Nothing) = throw new UnsupportedOperationException("A receive function cannot be called on the lowest layer.")
  
  def begin(): Unit
  def close(): Unit
  
}

object Serial {
  def open(port: String, baudRate: Int)(implicit provider: SerialProvider) = provider.open(port, baudRate)
}