# Fluids
![Fluids](screenshot.jpg)

This sample implements a fluid simulation.

## Maths

First some basic maths.

### Gradient

Given a scalar field defined by a function, $f( x, y, z )$, the gradient is a vector field defined by:

$$\nabla f = \frac{ \partial f }{ \partial x } \hat x + \frac{ \partial f }{ \partial y } \hat y + \frac{ \partial f }{ \partial z } \hat z $$

This describes the slope of the function and the direction of the change.

### Divergence

Given a vector field $F = ( F_x, F_y, F_z )$, the divergence is defined as:

$$\nabla \cdot F = \frac{ \partial F_x }{ \partial x } + \frac{ \partial F_y }{ \partial y } + \frac{ \partial F_z }{ \partial z }$$

You can think of this as how much the vector field spreads out or compresses.

### Curl

And the curl is given by:

$$\nabla \times F = \left( \frac{ \partial F_z }{ \partial y } - \frac{ \partial F_y }{ \partial z } \right) \hat x + \left( \frac{ \partial F_x }{ \partial z } - \frac{ \partial F_z }{ \partial x } \right) \hat y + \left( \frac{ \partial F_y }{ \partial x } - \frac{ \partial F_x }{ \partial y } \right) \hat z $$

This can be thought of how much the field rotates or swirls around.

### Laplacian

And finally the Laplacian of a scalar field is the divergence of the gradient:

$$\nabla \cdot \nabla f = \nabla^2 f = \frac{ \partial^2 f }{ \partial x^2 } + \frac{ \partial^2 f }{ \partial y^2 } + \frac{ \partial^2 f }{ \partial z^2 }$$

Which describes how "lumpy" the function is.  Basically, if the function describes a heightfield, then the Laplacian tells us where the peaks and vallies are located.

## Physics

Now for some physics.

### Gravity

As we all know $F = ma$ and the force of gravity is $F_g = mg$.  Therefore the acceleration due to gravity is:

$$a_g = g$$

### Pressure

Next up is pressure.  Pressure is force per unit area.  However, if the pressure is uniform, then it won't push a volume element.  But if there's a pressure gradient, then it will be able to push a volume element.

So, given a volume element (a cube) with side length $l$, and a pressure gradient in the x-direction, $\Delta p = p_{x_1} - p_{x_0}$, then force in the x-direction will be given by:

$$F_x = ma_x = l^3 \cdot \rho \cdot a = -\Delta p \cdot A = -\Delta p \cdot l^2 $$

This means that the acceleration in the x-direction is:

$$a_x = \frac{ -\Delta p }{ \rho l } = -\frac{ \partial p }{ \partial x } \frac{ 1 }{ \rho }$$

Which means the general acceleration for pressure will be:

$$a_p = -\frac{ \nabla p }{ \rho }$$

### Viscosity

Viscosity can be modeled as "blurring" the velocity field of a fluid.  Therefore the change in the velocity field can be written as:

$$v' = v + \Delta t \mu \nabla^2v$$

$v'$ is the new velocity, $v$ is the old velocity, $\Delta t$ is the change in time, and $\mu$ is a positive scaling factor.  Re-arranging the equation a little, we can get the acceleration:

$$a = \frac{ v' - v }{ \Delta t } = \mu \nabla^2v$$

### Density Constraint

If we are to simulate an incompressible fluid, then the density must remain constant $\rho = \rho_c$.  Which means the divergence must be zero.

$$\nabla \cdot v = 0$$

### Navier-Stokes

Combining all these equations gives us the Navier-Stokes equations for incompressible flows.

$$a = g - \frac{ \nabla p }{ \rho } + \mu \nabla^2 v$$
$$\nabla \cdot v = 0$$


## Lagrangian Framework

The Lagrangian framework is a simulation that represents fluid as a set of particles.

### Smoothed Particle Hydrodynamics

Smoothed Particle Hydrodynamics (SPH) was originally developed by astrophysicists.  The "smooth" terminology is because the particle is defined to be "fuzzy".

All Kernels are required to evaluate to 1 when integrated over all of space:

$$\int W(r) = 1$$

#### Smooth Kernel

$$
W_{std}(r)=\frac{315}{64\pi h^3}
\begin{cases}
(1-\frac{ r^2 }{ h^2 })^3 & \quad \text{when $0\leq r \leq h$}\\ 
0 & \quad \text{otherwise}
\end{cases}
$$

While the smooth kernel is perfectly suitable in a spatial sense, its first and second derivatives oscillate.  The oscillation of the derivatives can create unintentional attractive forces between particles.  We would like to avoid this, and a different kernel has been developed to handle this problem.

#### Spikey Kernel

$$
W_{spikey}(r)=\frac{15}{\pi h^3}
\begin{cases}
(1-\frac{ r }{ h })^3 & \quad \text{when $0\leq r \leq h$}\\ 
0 & \quad \text{otherwise}
\end{cases}
$$

The derivatives of the spikey kernel do not oscillate and lead to only repulsive forces.

#### Interpolation

Now that we have a kernel that can define a particle.  We need to be able to interpolate between particles:

$$\phi( x ) = m \sum_j \frac{ \phi_j }{ \rho_j } W( x - x_j )$$

Where $\phi$ is the quantity that we want to interpolate.  All the other values should be self explanatory.

#### Density

Density can be described with the following equation:

$$\rho( x ) = m \sum_j W( x - x_j )$$

#### Gradient

$$\nabla \phi( x ) = m \sum_j \frac{ \phi_j }{ \rho_j } \nabla W( \vert x - x_j \vert )$$

However, this version of the gradient can be asymetric.  So two particles may calculate different values.  Which will cause different opposing forces.  So, an alternate gradient can be defined as:

$$\nabla \phi( x ) = \rho_i m \sum_j \left( \frac{ \phi_i }{ \rho_i^2 } + \frac{ \phi_j }{ \rho_j^2 } \right) \nabla W( \vert x - x_j \vert )$$

#### Laplacian

$$\nabla^2 \phi( x ) = m \sum_j \left( \frac{ \phi_j - \phi_i }{ \rho_j } \right) \nabla^2 W( x - x_j )$$

#### Solver

The basic solver is to:

- Calculate the densities
- Calculate pressure from density
- Calculate pressure gradient force
- Calculate viscosity force
- Calculate gravity
- Step time

#### Pressure

$$p = \frac{ \kappa }{ \gamma } ( \frac{ \rho }{ \rho_0 } - 1 )^{\gamma}$$

$$\kappa = \rho_0 \frac{ c_s }{ \gamma }$$

Here, $c_s$ is the speed of sound (a constant in our simulation), and $\gamma$ is a tuning parameter.

#### Pressure Gradient

$$f_p = -m^2 \sum_j \left( \frac{ p_i }{ \rho_i^2 } + \frac{ p_j }{ \rho_j^2 } \right) \nabla W( x - x_j )$$

#### Viscosity

$$f_v = -m \sum_j \left( \frac{ u_j - u_i }{ \rho_j } \right) \nabla^2 W( x - x_j )$$

#### Collision

As for handling collision with external geometry, the collision can be resolved using the projection method.  The velocity normal to the surface is negated and reduced by the coefficient of restitution, and the tangential velocity is scaled by the friction.

#### Sorting

This simulation uses a bitonic sorting algorithm that was adapted from Sebastian Lague's "Fluid Simulation".

As a note to myself, for games, where perfect accuracy isn't necessary.  The bitonic sort could probably be replaced with an incomplete sorting algorithm.  If the fluid particles don't move too much from frame to frame, and the grid is organized in memory so that each neighboring cell is nearby (morton order perhaps).  Then a complete sort wouldn't be required each frame, since the particles are already mostly sorted.  This should significantly speed up the simulation.

## Eulerian Framework

The Eulerian framework is a simulation that represents fuild with a grid.

The grids can be square or cubic.  But they don't have to be.  There's also curvilinear grids and irregular or triangulated/tetrahedral grids.  The grids can also be body, face, edge, or vertex centered.  And they may contain scalar or vector data.

### Differential Operators

#### Finite Difference

Forward difference:

$$\frac{ \partial f }{ \partial x } = \frac{ f^{i+1, j, k} - f{i, j, k} }{ \delta x }$$

Backward difference:

$$\frac{ \partial f }{ \partial x } = \frac{ f^{i, j, k} - f{i-1, j, k} }{ \delta x }$$

Center difference:

$$\frac{ \partial f }{ \partial x } = \frac{ f^{i+1, j, k} - f{i-1, j, k} }{ 2 \delta x }$$

For the second order derivative, let's first define $g$:

$$g^{i+1/2, j, k } = \frac{ f^{i+1, j, k} - f^{i, j, k} }{ \delta x }$$

$$g^{i-1/2, j, k } = \frac{ f^{i, j, k} - f^{i-1, j, k} }{ \delta x }$$

$$\frac{ \partial^2 f }{ \partial x^2 } = \frac{ g^{i+1/2, j, k} - g^{i-1/2, j, k }{ \delta x }$$
$$= \frac{ f^{i+1, j, k} - 2f^{i, j, k} + f^{i-1, j, k} }{ \delta x^2 }$$

Gradient:

$$\nabla f(x) = \left( \frac{ f^{i+1, j, k} - f{i-1, j, k} }{ 2 \delta x }, \frac{ f^{i, j+1, k} - f{i, j-1, k} }{ 2 \delta y }, \frac{ f^{i, j, k+1} - f{i, j, k-1} }{ 2 \delta z } \right)$$

Divergence:

$$\nabla \cdot F(x) = \frac{ F_x^{i+1, j, k} - F_x{i-1, j, k} }{ 2 \delta x } + \frac{ F_y^{i, j+1, k} - F_y{i, j-1, k} }{ 2 \delta y } + \frac{ F_z^{i, j, k+1} - F_z{i, j, k-1} }{ 2 \delta z }$$

Curl:

$$\nabla \cross F(x) = \left( \frac{ F_z^{i, j+1, k} - F_z{i, j-1, k} }{ \delta y } - \frac{ F_y^{i, j, k+1} - F_y{i, j, k-1} }{ \delta z } \right) \hat x$$
$$+ \left( \frac{ F_x^{i, j, k+1} - F_x{i, j, k-1} }{ \delta z } - \frac{ F_z^{i+1, j, k} - F_z{i-1, j, k} }{ \delta x } \right) \hat y$$
$$+ \left( \frac{ F_y^{i+1, j, k} - F_y{i-1, j, k} }{ \delta x } - \frac{ F_x^{i, j+1, k} - F_x{i, j-1, k} }{ \delta y } \right) \hat z$$

Laplacian:

$$\nabla^2 f(x) = \frac{ f^{i+1, j, k} - 2f^{i, j, k} + f^{i-1, j, k} }{ \delta x^2 }$$
$$+ \frac{ f^{i, j+1, k} - 2f^{i, j, k} + f^{i, j-1, k} }{ \delta y^2 }$$
$$+ \frac{ f^{i, j, k+1} - 2f^{i, j, k} + f^{i, j, k-1} }{ \delta z^2 }$$

### Simulation

#### Collision Handling

The grid based system uses signed distance fields to handle collision.

The boundary condition at the surface is $u \cdot n = 0$, this is the no flux boundary condition, as it requires the velocity to be tangent to the surface.

This can be extended to scalar fields with the Neumann and Dirichlet conditions.

Neumann:

$$\frac{ \partial f }{ \partial n } = c$$

Dirichlet:

$$f = c$$

There's also slip/no-slip boundary conditions that affect the tangent component of the velocity field:

$$u_t = max \left( 1 - \mu \frac{ max( -u \cdot n, 0 ) }{ \abs{ u_t } } \right) u_t$$

#### Advection

In order to integrate the simulation forward, we actually need to step backwards.  This is because the grid cells are calculated at a specific location, but the flows do not necessarily go from grid point to point.  Since we can sample the grid anywhere and lerp between values, we calculate the grid at each specific point and use the past for sampling.  See "Fluid Engine Development", section 3.4.2.1 Semi-Lagrangian Method for an illustration in Figure 3.10.

$$f(x)^{n+1} = f(x - \delta t u )^n$$

The advection equation is:

$$\frac{ \partial f }{ \partial t } + u \cdot \nabla f = 0$$

For one dimension:

$$\frac{ \partial f }{ \partial t } = -u \frac{ \partial f }{ \partial x }$$

And using the Euler method to approximate it:

$$f_i^{t+\delta t} = f_i^t - \delta t u \frac{ f_i^t - f_{i-1}^t }{ \delta x }$$

This can have some accuracy issues in circular flows.  And to improve accuracy, you can use the midpoint method.  The midpoint method uses a half time step to go backwards, and then resamples the velocity, using the resample velocity to make the full step backward to get the previous sample point.

Interpolation can be improved by using Catmull-Rom interpolation instead of linear interpolation.

#### Diffusion/Viscosity

$$a_v = \mu \nabla^2 u$$

This can be integrated with Euler integration:

$$u^{n+1} = u^n + \delta t \mu \nabla^2 u^n$$

Improving the accuracy using backward Euler:

$$f_{i,j,k}^{n+1} = f_{i,j,k}^n + \delta t \mu L( f_{i,j,k}^n )$$

Where $L$ is the central differencing defined above.

In order to further increase the accuracy of the simulation.  The entire grid needs to be notified of all the changes at each point, all at once.  This requires build a large matrix that results in the linear complimentary problem:

$$A \cdot x = b$$

If the diffusion coefficient and time steps are high, then this can be solved with conjugate gradient methods.  Otherwise, this can be solved using Gauss-Seidel or Jacobi methods.

#### Pressure

$$a_p = - \frac{ \nabla p }{ \rho }$$

Euler method gives us:

$$u^{n+1} = u^n - \delta t \frac{ \nabla p }{ \rho }$$

This will also result in a large matrix that needs to be solved using LCP methods.

#### Buoyancy Force

The buoyancy force can be described with the following equation:

$$\nabla \cdot \frac{ \nabla p }{ \rho } = c \frac{ \nabla \cdot u }{ \delta t }$$

## Hybrid Methods

### Particle-in-Cell

The idea of the particle in the cell is to update positions using particles and to update velocities using grid cells.  The method involves the following steps:

- Transfer particle velocities into grid cells
- Compute non-advection forces
- Transfer velocities back to particles
- Move the particles

### Fluid-Implicit Particle Method (FLIP)

FLIP is an adaptation of the PIC method.  The difference is that the PIC method interpolates the velocities when transfer them from the grid to the particle, and the FLIP method interpolates the velocity deltas instead.


### Other methods

There's also Particle Level Set and Vortex Particle.  It's also good to remember that fluid simulation is an area of active research and there's always new improvements, such as Sparse Voxel Grids.


## References

"Particle Simulation using CUDA" by Simon Green

"Fluid Engine Development" by Doyub Kim

"Smoothed Particle Hydrodynamics Techniques for the Physics Based Simulation of Fluids and Solids", Dan Koschier

"Particle-based Viscoelastic Fluid Simulation", Simon Clavet

"Particle-based Fluid Simulation for Interactive Applications", Mathias Muller

"Fluid Simulation" by Sebastian Lague

"Fast Fluid Simulations with Sparse Volumes on the GPU", Kui Wu






