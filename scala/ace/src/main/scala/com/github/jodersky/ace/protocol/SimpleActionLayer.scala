package com.github.jodersky.ace.protocol

class SimpleActionLayer[A](action: A => Unit) extends ReactiveLayer[A, A] {
  protected def receive(data: A) = action(data)
  
  def send(data: A) = sendToLower(data)

}