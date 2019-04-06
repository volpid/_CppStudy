rem set GlslangValidator=D:\_SubDevTools\VulkanSDK\1.0.68.0\Bin\glslangValidator.exe
set GlslangValidator=D:\DevelopmentTools\VulkanSDK\1.1.97.0\Bin\glslangValidator.exe

%GlslangValidator% -V 01_shader.vert -o 01_shader.vert.spv
%GlslangValidator% -V 01_shader.frag -o 01_shader.frag.spv

%GlslangValidator% -V 02_shader.vert -o 02_shader.vert.spv
%GlslangValidator% -V 02_shader.frag -o 02_shader.frag.spv
%GlslangValidator% -V 02_shader.geom -o 02_shader.geom.spv

%GlslangValidator% -V 03_shader.vert -o 03_shader.vert.spv
%GlslangValidator% -V 03_shader.frag -o 03_shader.frag.spv
%GlslangValidator% -V 03_shader.comp -o 03_shader.comp.spv
%GlslangValidator% -V 03_shader.geom -o 03_shader.geom.spv

%GlslangValidator% -V 04_shader.vert -o 04_shader.vert.spv
%GlslangValidator% -V 04_shader.frag -o 04_shader.frag.spv
%GlslangValidator% -V 04_shader.tesc -o 04_shader.tesc.spv
%GlslangValidator% -V 04_shader.tese -o 04_shader.tese.spv
%GlslangValidator% -V 04_shader.geom -o 04_shader.geom.spv

%GlslangValidator% -V 05_shader.vert -o 05_shader.vert.spv
%GlslangValidator% -V 05_shader.frag -o 05_shader.frag.spv

%GlslangValidator% -V 06_model.vert -o 06_model.vert.spv
%GlslangValidator% -V 06_model.frag -o 06_model.frag.spv
%GlslangValidator% -V 06_skybox.vert -o 06_skybox.vert.spv
%GlslangValidator% -V 06_skybox.frag -o 06_skybox.frag.spv
%GlslangValidator% -V 06_postprocess.vert -o 06_postprocess.vert.spv
%GlslangValidator% -V 06_postprocess.frag -o 06_postprocess.frag.spv

pause
