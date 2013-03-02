package com.github.jodersky.ace

trait PhysicalLayer extends ReactiveLayer[Nothing, Seq[Int]] {
  protected def receive(nothing: Nothing) = throw new UnsupportedOperationException("A receive function cannot be called on the lowest layer.")
}