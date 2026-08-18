CS 499 Milestone Two: Enhancement One - Software Design and Engineering
Student: Arishia Jackson
Original artifact: CS 330 Computational Graphics and Visualization Final Project

ARTIFACT DESCRIPTION
The program is a C++ OpenGL application that renders a ground plane, an animated
3-by-3-by-3 array of colored cubes, and a pink cylindrical bottle with a textured cap.

COMPLETED ENHANCEMENTS
1. Refactored the scene into RenderGroundPlane, RenderAnimatedCubeArray, and
   RenderBottle methods.
2. Added a camera reset command and mouse-wheel camera-speed adjustment.
3. Added runtime controls for enabling/disabling the spotlight and adjusting intensity.
4. Added initialization, null-pointer, texture, material, and resource error messages.
5. Initialized texture state safely and added OpenGL texture cleanup in the destructor.
6. Improved naming, constants, comments, and separation of responsibilities.

CONTROLS
W/A/S/D       Move forward, left, backward, and right
Q/E           Move down and up
Mouse         Look around the scene
Mouse wheel   Decrease or increase movement speed
P/O           Perspective or orthographic projection
R             Reset camera position, direction, speed, and projection
1/2           Turn the primary spotlight on or off
3/4           Increase or decrease spotlight intensity (0.0 to 2.0)
Escape        Close the application

IMPORTANT PROJECT NOTE
The Visual Studio project references the SNHU starter files ShapeMeshes.cpp,
ShaderManager.cpp, camera.h, stb_image.h, shader files, and library folders outside
this submitted project directory. Keep the project in the same SNHU course workspace
structure used for the original CS 330 project so those relative paths remain valid.

VALIDATION PERFORMED
- Compared the enhanced source against the original source to preserve every original
  scene object and transformation.
- Verified that all declarations have matching definitions.
- Verified that the two submitted texture files are retained.
- Verified that lighting intensity is clamped between 0.0 and 2.0.
- Verified that one-time key presses are debounced for reset and lighting changes.

A full compile/run test must be performed in the original Windows Visual Studio/SNHU
workspace because this ZIP does not contain the external SNHU Utilities, 3DShapes,
GLFW, GLEW, GLM, camera, stb_image, or shader source dependencies referenced by the
Visual Studio project.
