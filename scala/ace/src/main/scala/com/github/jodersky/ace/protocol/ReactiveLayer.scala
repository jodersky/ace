package com.github.jodersky.ace.protocol

import scala.concurrent.Future

/** Represents a layer in a reactive protocol.
 *  @tparam L data type this layer receives from or writes to a lower layer
 *  @tparam T data type this layer sends to a higher layer or receives from a higher */
trait ReactiveLayer[L, T] {
  private var lowerLayer: Option[ReactiveLayer[_, L]] = None 
  private var higherLayer: Option[ReactiveLayer[T, _]] = None
  
  /** Notifies a higher layer that data is available. */
  protected def notifyHigher(data: T): Unit = higherLayer match {
    case Some(higher) => higher.receive(data)
    case None => throw new UnsupportedOperationException("Higher layer doesn't exist.")
  }
  
  /** Writes data to a lower layer. */
  protected def writeToLower(l: L): Future[L] = lowerLayer match {
    case Some(lower) => lower.write(l)
    case None => Future.failed(new UnsupportedOperationException("Lower layer doesn't exist."))
  }
  
  /** Connects this layer with a higher layer, effectively linking calls
   *  `notifyHigher` to `higher.receive` and `higher.writeToLower` to `write`. */
  def connect[A](higher: ReactiveLayer[T, A]) = {
    this.higherLayer = Some(higher)
    higher.lowerLayer = Some(this)
    higher
  }
  
  /** Called from lower layer. */
  def receive(data: L): Unit
  
  /** Write data to this layer.
   *  @return a future value containing the data written, or an error */
  def write(data: T): Future[T]
}