# Software Design and Engineering Narrative

## Artifact Description

The artifact selected for the software design and engineering category is my final project from CS 330: Computational Graphics and Visualization. I created the original project in spring 2025. It is a C++ OpenGL application that renders a three-dimensional scene containing a large ground plane, an animated 3-by-3-by-3 array of colored cubes, and a pink cylindrical bottle with a textured cap. The application uses GLFW to create the display window and process input, GLEW to access OpenGL functions, GLM for mathematical transformations, shader programs for rendering, texture images for the bottle cap, reusable mesh classes, and an interactive camera.

The uploaded artifact contains MainCode.cpp, SceneManager.cpp and SceneManager.h, ViewManager.cpp and ViewManager.h, the Visual Studio solution and project files, two texture files, and the compiled release executable from the original submission. The Visual Studio project also references SNHU starter resources stored outside the submitted folder, including ShapeMeshes.cpp, ShaderManager.cpp, camera.h, stb_image.h, shader files, and the GLFW, GLEW, and GLM libraries.

## Justification for Inclusion and Enhancement

I selected this artifact because it demonstrates several connected software engineering skills in one working graphics application. The project required C++ programming, object-oriented design, real-time rendering, transformations, camera movement, texture loading, materials, lighting, user input, and resource management. The animated scene provides visible evidence of the program’s behavior, while the source code demonstrates how the application is organized and implemented.

The first enhancement was a structural refactor of the scene-rendering code. In the original SceneManager.cpp file, the ground plane, cube array, bottle body, and bottle cap were all rendered inside one RenderScene method. In the enhanced version, the scene is divided into RenderGroundPlane, RenderAnimatedCubeArray, and RenderBottle methods. RenderScene now coordinates those three responsibilities. This change preserves the original objects and transformations while making each part of the scene easier to locate, test, change, and reuse.

The second enhancement improved the camera system. The original project already supported W, A, S, and D movement, vertical movement with Q and E, mouse-look controls, and perspective and orthographic projection. I added an R-key reset command that restores the original camera position, direction, zoom, movement speed, and perspective projection. I also added a mouse-wheel callback that adjusts movement speed and limits that speed to a safe range between 0.5 and 10.0. These additions improve usability and prevent extreme speed values.

The third enhancement made the spotlight configurable while the program is running. The 1 key enables the primary spotlight, the 2 key disables it, the 3 key increases its intensity, and the 4 key decreases its intensity. The intensity is limited to a range from 0.0 to 2.0. Key-state tracking prevents a single key press from repeatedly changing the setting during every frame. The lighting update method applies the current enabled state and intensity to the spotlight’s ambient, diffuse, and specular shader values.

The fourth enhancement strengthened defensive programming and error reporting. The enhanced program checks GLFW initialization, window creation, GLEW initialization, manager availability, texture capacity, texture loading, OpenGL texture creation, unsupported texture channel counts, missing texture tags, and missing material tags. It now prints specific error or warning messages that identify the failed operation or resource. The texture counter is initialized to zero, texture IDs are deleted during cleanup, and the main application destroys managers, the display window, and GLFW resources in a controlled order.

The final enhancement improved readability and maintainability. I replaced several raw values with named constants, used constructor initializer lists, applied nullptr instead of NULL, added const references where appropriate, clarified file headers and method responsibilities, added a control guide, and created a README that identifies the original scene, completed enhancements, controls, dependency requirements, and validation status.

## Course Outcomes

This enhancement demonstrates substantial progress toward Course Outcomes Two, Three, Four, and Five.

Course Outcome Two requires professional-quality oral, written, and visual communication that is coherent, technically sound, and adapted to its audience. The enhanced artifact supports this outcome through its interactive visual scene, clearer source-code organization, descriptive error messages, comments, README instructions, and this narrative. Together, these elements help an instructor, employer, or developer understand what the program does, how to control it, and how the enhancement improved it.

Course Outcome Three requires the design and evaluation of computing solutions using algorithmic principles, computer science practices, and appropriate design trade-offs. I addressed this outcome by evaluating the original large RenderScene method and separating it according to object responsibility. The refactor adds more methods, but the trade-off is improved readability, isolation of changes, and maintainability. I also used bounded values and key-state logic to manage camera speed and lighting changes predictably.

Course Outcome Four requires the use of well-founded and innovative computing techniques, skills, and tools to implement solutions that deliver value. The enhanced project uses C++, OpenGL, GLFW, GLEW, GLM, shaders, textures, object-oriented design, callbacks, real-time input processing, and reusable rendering methods. The enhancement delivers value by making the application more interactive, easier to navigate, safer to operate, and easier for another developer to understand or extend.

Course Outcome Five requires a security mindset that anticipates software flaws, exposes vulnerabilities, mitigates design weaknesses, and protects data and resources. This graphics artifact does not process confidential data, but the enhancement applies the same defensive mindset through validation, bounded input values, null checks, resource-capacity checks, specific failure messages, and controlled texture and application cleanup. These changes reduce the risk of crashes, invalid program states, and unmanaged resources.

The outcome-coverage plan remains consistent with the plan created in Module One. The enhancement provides the strongest evidence for Outcomes Three and Four, while the improved documentation supports Outcome Two and the added validation and cleanup practices support Outcome Five.

## Reflection on the Enhancement Process

Enhancing this artifact showed me that successful visual output does not automatically mean that a program has strong software engineering quality. The original project rendered the intended scene, but the review revealed that its main scene method combined several separate responsibilities. Refactoring the method required me to trace each transformation, material, color, texture, and mesh call so that I could move the code without changing the original scene.

One challenge was working with an artifact that depends on course starter files and libraries located outside the submitted folder. The Visual Studio project references ShapeMeshes.cpp, ShaderManager.cpp, shader files, camera and image-loading headers, and OpenGL libraries through relative paths and a local vcpkg installation. I preserved those project references instead of pretending the submitted ZIP was fully standalone. I also documented that the final compile and runtime test must be performed in the original Windows Visual Studio and SNHU workspace where those dependencies are available.

Another challenge was adding input features to a continuously running render loop. Without key-state tracking, holding a lighting key for a fraction of a second could change the intensity many times because input is evaluated every frame. I addressed this by storing the previous state of each lighting key and applying a change only when the key transitions from released to pressed. I used the same approach for the camera reset command.

The texture code also revealed a design flaw in the original artifact: m_loadedTextures was declared but not initialized in the constructor. Because the value controls where texture information is stored, leaving it uninitialized could result in unpredictable behavior. Initializing it to zero and checking the maximum texture capacity improved reliability. I also learned that error paths must release any resources already created, such as image memory and OpenGL texture IDs, before returning.

Overall, the enhancement strengthened my understanding of modular design, state management, callbacks, input validation, resource cleanup, error handling, and honest technical documentation. The final artifact preserves the original scene while presenting a clearer and more professional example of my software design and engineering abilities.

## Artificial Intelligence Use Acknowledgment

I used OpenAI’s ChatGPT to help review the assignment requirements, inspect and organize the uploaded project files, support the code-refactoring process, and refine the written narrative. I reviewed the enhanced source files and narrative to ensure that they describe the uploaded artifact and the changes included in the submission. The narrative also identifies the external project dependencies and does not claim that a complete compile and runtime test occurred in an environment where those dependencies were unavailable.
