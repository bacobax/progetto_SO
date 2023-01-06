calcolaTempoViaggio <- function(x,y, destX, destY, velocita){
  deltaX <- destX - x
  deltaY <- destY - y
  
  distanza <- sqrt(deltaX^2 + deltaY^2)
  round(distanza / velocita , digits=6)
}


tempo <- calcolaTempoViaggio(4,0,0,4,0.7)

