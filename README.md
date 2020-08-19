# Fermat
### Note
To successfully compile this code, you need to have Qt installed on your system. A simple option is to download the [Qt Creator IDE](https://www.qt.io/product/development-tools).
### What this project is about
Let's assume we want to know which path a classical particle with conserved energy takes between a fixed beginning and ending point. It turns out that, among all paths <img src="https://latex.codecogs.com/svg.latex?\Gamma" /> that the particle _could_ take between the two points, the one prescribed by classical mechanics is the one that minimizes the __abbreviated action__ functional

<img src="https://latex.codecogs.com/svg.latex?\int_\Gamma%20\!%20\vec{p}(\vec{x})%20\,%20\mathrm{d}\vec{s}%20=%20\int_\Gamma%20\!%20\sqrt{2m(E-V(\vec{x}))}%20\,%20\mathrm{d}s" />

This is known as [__Maupertuis' principle__](https://en.wikipedia.org/wiki/Maupertuis%27s_principle).


In optics, something suspiciously similar is going on. [__Fermat's principle__](https://en.wikipedia.org/wiki/Fermat%27s_principle) tells us that a light ray takes the path between two points that needs the least amount of time to traverse. As light moves at an <img src="https://latex.codecogs.com/svg.latex?n"/>-th fraction of its vacuum speed given a refractive index <img src="https://latex.codecogs.com/svg.latex?n"/>, satisfying Fermat's principle amounts to minimizing the __optical path length__

<img src="https://latex.codecogs.com/svg.latex?\int_\Gamma%20\!%20n(\vec{x})%20\,%20\mathrm{d}s" />

Here comes the interesting part: we can exploit this correspondence and convert any ray optics problem into one from mechanics. Take the following setting: say we assign a refractive index to any point in space. This distribution could model some type of lens, a boundary between two optical media, or anything else we're curious about. At some point we place a light source that emits rays into multiple directions. Now let's forget about optics altogether. Fix some arbitrary, but constant energy value <img src="https://latex.codecogs.com/svg.latex?E" /> and construct a mechanical potential out of <img src="https://latex.codecogs.com/svg.latex?n" /> by the prescription

<img src="https://latex.codecogs.com/svg.latex?V(\vec{x})%20=%20E%20-%20\frac{n(\vec{x})^2}{2m}" />

Next, let a number of particles (all with energy <img src="https://latex.codecogs.com/svg.latex?E" />) leave the source location in different directions and find their trajectories in the potential by numerically integrating Newton's second law. Because of the way we constructed <img src="https://latex.codecogs.com/svg.latex?V" /> out of <img src="https://latex.codecogs.com/svg.latex?n" />, these trajectories do not only satisfy Maupertuis' principle in their potential, but also Fermat's principle in the refractive index distribution that defined our original problem. The particles move just like the light rays that we want to trace!


While this is great to know in theory, I thought it might be even better to have something interactive to play around with. This is why I wrote this program which is two things in one: __a Newtonian particle and light ray simulation, as these are just two physical perspectives on the same mathematical problem__. After entering a function for the refractive index distribution, you can either take the viewpoint of simulating light rays in this distribution, or you can switch to a program mode that displays the derived potential, in which the same plotted rays have the meaning of particle trajectories. Let's see how this works out in practice.
