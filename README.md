# LunarLander

The objective of this project was to implement forces on our lander such as gravity, thruster, and turbulence to simulate a landing of our lunar lander onto our terrain. 

![](https://github.com/LPx1/MoonLander/blob/master/lunarlander.png)

To do this, I casted rays down to the surface of the terrain and using an octree to test for intersection between the ray casted from the lander to the surface. While casting the ray down for an intersection, it is also calculating the distance from the lander to the surface. 

In addition to forces being enacted onto the lander such as gravity pulling the lander down to the surface on the terrain. The turbulence force generating random values of where it will pull the lander. Lastly, thruster force will push the lander whether it be forward, back, up, down, left or right through using the arrow keys for direction. 

Also implemented are multiple cameras ranging from cameras following the lander or cameras placed looking at a general direction.

Video Demonstrating the features of the project:

https://www.youtube.com/watch?v=es8I6xOLN9M
