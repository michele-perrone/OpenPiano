# Open Piano: an open source piano engine based on physical modeling

![](Documentation/Images/openpiano_screenshot.png)

## Summary
### [1. What is Open Piano?](#what-is-open-piano)
### [2. How do I use it?](#how-does-it-work)
### [3. Current state and road map](#current-state-and-road-map)
### [4. How does it work?](#how-do-i-use-it)
### [5. Can I contribute to this project?](#can-i-contribute-to-this-project)
### [6. Are you crazy?](#are-you-crazy)


## What is Open Piano?
Open Piano is an attempt at recreating the characteristic sound of the piano with physical modeling sound synthesis. This allows for greater expressiveness and flexibility in comparison to sampled pianos. There is also no need to download huge libraries of audio samples, since everything is generated in real time.

## How do I use it?
**DISCLAIMER:** I don't discourage you in trying out Open Piano, but please note that this is still pre-alpha quality software. It is nowhere near, and will probably never be, commercial plugins like Modartt's [Pianoteq](https://www.modartt.com/pianoteq) (which I own, and love) and Arturia's [Piano V](https://www.arturia.com/products/analog-classics/piano-v/overview). A first binary release of Open Piano will be published once a usable state has been reached. 

## Current state and road map
Right now, Open Piano sounds like a strangely out of tune two-octave piano with no soundboard, only one string per note, and no pedal. And it eats too much CPU. What needs to be done:
- Optimize the FD model to make it... usable. Take advantage of multithreading. In case FD shows itself to be too burdensome, consider the possibility of switching to modal analysis
- Introduce multiple strings per note and simulate the double decay phenomenon
- Introduce the pedal
- Simulate the soundboard

## How does it work?
Everything starts from the differential equation of vibration of a stiff string, hit by a hammer:

![](Documentation/Images/stiff_string_differential_equation.svg)

To obtain the spatial displacement of each piano string at each temporal instant, we need to solve this equation. There are different approaches: two examples are finite differences (FD) and modal analysis. The idea behind FD is to discretize the differential equation by substituting its derivatives with finite differences - hence the name. Modal analysis, on the other hand, assumes that the solutions of the equation are in modal form, and discretizes the solutions rather than the equation itself. Open Piano uses the FD approach.

## Can I contribute to this project?
Once the project reaches a certain usability level, contributions will be welcome.

## Are you crazy?
Yes I do. I am. Whatever