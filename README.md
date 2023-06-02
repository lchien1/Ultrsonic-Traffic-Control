# Ultrasonic Traffic Controller

Created by Lonnie Chien, Quinn Okabayashi, Cara Lee

Design (description by Professor Maggie Delano)
The T intersection appears as shown in the image below. Cars entering the intersection from the bottom (car in blue) may only make a right hand turn onto the street. Cars on the busier road (cars in red) can only go straight, either to the left or the right depending on the lane.  

![[Image source](https://static.epermittest.com/media/filer_public/26/f3/26f320b9-af42-409d-8e4a-9d406b717c3d/2-right-of-way-at-uncontrolled-t-intersection.png)](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/c026802d-8242-417b-9ed9-d1722be0f05e/2-right-of-way-at-uncontrolled-t-intersection.png)

[Image source](https://static.epermittest.com/media/filer_public/26/f3/26f320b9-af42-409d-8e4a-9d406b717c3d/2-right-of-way-at-uncontrolled-t-intersection.png)

Your traffic control scheme should monitor traffic flowing to the right and traffic flowing from the bottom and use this information to control two stoplights: one stoplight controls the blue traffic, and the other controls the red traffic.

The control scheme should work as follows:

| Scenario | Red Lane Behavior | Blue Lane Behavior |
| --- | --- | --- |
| Red cars detected, no blue cars detected | Green light | Red light |
| Blue cars detected, no red cars detected | Red light | Green light |
| Both types of cars detected | On for two minutes, off for one minute | Off for two minutes, on for one minute |
| No cars detected | Green light | Red light |

Also implement the following conditions for safety reasons:

- ensure that the two stoplights never have two green lights on at the same time
- when transitioning from a green light to a red light, you should turn on a yellow light for 5 seconds in between, and there should be a period of at least 5 more seconds where both lights are red before a different stoplight turns green
