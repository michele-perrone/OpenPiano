# Open Piano: an open source piano engine based on physical modeling

![](Documentation/Images/openpiano_screenshot.png)

## Summary
### [1. What is Open Piano?](#what-is-open-piano)
### [2. How do I use it?](#how-do-i-use-it)
### [3. Current state and road map](#current-state-and-road-map)
### [4. How does it work?](#how-does-it-work)
### [5. Can I contribute to this project?](#can-i-contribute-to-this-project)
### [6. Are you crazy?](#are-you-crazy)


## What is Open Piano?
Open Piano is an attempt at recreating the characteristic sound of the piano with physical modeling sound synthesis. This allows for greater expressiveness and flexibility in comparison to sampled pianos. There is also no need to download huge libraries of audio samples, since everything is generated in real time.

## How do I use it?
**DISCLAIMER:** I don't discourage you from trying out Open Piano, but please note that this is still alpha quality software. As of now, it is nowhere near commercial plugins like Modartt's [Pianoteq](https://www.modartt.com/pianoteq) (which I own, and love) or Arturia's [Piano V](https://www.arturia.com/products/analog-classics/piano-v/overview). A first binary release of Open Piano will be published once the project reaches a usable state. 

## Current state and road map
Right now, Open Piano sounds like a strangely out of tune piano with no soundboard, only one string per note, and no pedal. Here's a short list of what needs to be done:
- ~~Fix repeated hits of the string by the hammer, without resetting the entire string displacement (see [here](https://github.com/michele-perrone/OpenPiano/blob/c338f46ce50802265661e2898c5619e9c2654629/OpenPianoCore/string_hammer.h#L307))~~ - [FIXED](https://github.com/michele-perrone/OpenPiano/commit/d0461f860075b43f8b4d246c1d99371dc0ab606f) 
- Optimize the FD model to make it... usable. Take advantage of multithreading (being worked on, see [here](https://github.com/michele-perrone/OpenPiano/commit/eb89378566dbc875619000024de95a31c819be7c) and [here](https://github.com/michele-perrone/OpenPiano/commit/e0e39ef8e5ea7fce7f0421fb5f2a5fab7b39b1df))
- Introduce the pedal and string damping
- Introduce multiple strings per note and simulate the double decay phenomenon
- Simulate the soundboard
- Find a decent set of physical parameters for all the strings

In case FD shows itself to be too burdensome, consider the possibility of switching to modal analysis.

## How does it work?
Everything starts from the differential equation of vibration of a lossy stiff string, hit by a hammer:

![](Documentation/Images/stiff_string_differential_equation.png)

To obtain the spatial displacement of each piano string at each temporal instant, we need to solve this equation. There are different approaches: two examples are finite differences (FD) and modal analysis. The idea behind FD is to discretize the differential equation by substituting its derivatives with finite differences - hence the name. Modal analysis, on the other hand, assumes that the solutions of the equation are in modal form, and discretizes the solutions rather than the equation itself. Open Piano uses the FD approach.

## Can I contribute to this project?
Once the project reaches a certain usability level, contributions will be welcome.

## Are you crazy?
Yes I do. I am. Whatever