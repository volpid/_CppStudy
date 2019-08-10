set GlslangValidator=D:\DevelopTools\VulkanSDK\1.0.68.0\Bin\glslangValidator.exe
rem set GlslangValidator=D:\DevelopmentTools\VulkanSDK\1.1.97.0\Bin\glslangValidator.exe

%GlslangValidator% -V 01_shader.vert -o 01_shader.vert.spv
%GlslangValidator% -V 01_shader.frag -o 01_shader.frag.spv

%GlslangValidator% -V 02_shader.vert -o 02_shader.vert.spv
%GlslangValidator% -V 02_shader.frag -o 02_shader.frag.spv

%GlslangValidator% -V 03_shader.vert -o 03_shader.vert.spv
%GlslangValidator% -V 03_shader.frag -o 03_shader.frag.spv

%GlslangValidator% -V 04_model.vert -o 04_model.vert.spv
%GlslangValidator% -V 04_model.frag -o 04_model.frag.spv
%GlslangValidator% -V 04_skybox.vert -o 04_skybox.vert.spv
%GlslangValidator% -V 04_skybox.frag -o 04_skybox.frag.spv

%GlslangValidator% -V 05_scene.vert -o 05_scene.vert.spv
%GlslangValidator% -V 05_scene.frag -o 05_scene.frag.spv
%GlslangValidator% -V 05_shadow.vert -o 05_shadow.vert.spv

pause
