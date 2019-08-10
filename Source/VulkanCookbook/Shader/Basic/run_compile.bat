set GlslangValidator=D:\DevelopTools\VulkanSDK\1.0.68.0\Bin\glslangValidator.exe
rem set GlslangValidator=D:\DevelopmentTools\VulkanSDK\1.1.97.0\Bin\glslangValidator.exe

%GlslangValidator% -V 04_shader.vert -o 04_shader.vert.spv
%GlslangValidator% -V 04_shader.frag -o 04_shader.frag.spv

%GlslangValidator% -V 05_shader.vert -o 05_shader.vert.spv
%GlslangValidator% -V 05_shader.frag -o 05_shader.frag.spv

%GlslangValidator% -V 06_shader.vert -o 06_shader.vert.spv
%GlslangValidator% -V 06_shader.frag -o 06_shader.frag.spv

%GlslangValidator% -V 07_shader.vert -o 07_shader.vert.spv
%GlslangValidator% -V 07_shader.frag -o 07_shader.frag.spv

%GlslangValidator% -V 08_shader.vert -o 08_shader.vert.spv
%GlslangValidator% -V 08_shader.frag -o 08_shader.frag.spv
%GlslangValidator% -V 08_shader.tesc -o 08_shader.tesc.spv
%GlslangValidator% -V 08_shader.tese -o 08_shader.tese.spv

%GlslangValidator% -V 09_shader.vert -o 09_shader.vert.spv
%GlslangValidator% -V 09_shader.frag -o 09_shader.frag.spv
%GlslangValidator% -V 09_shader.geom -o 09_shader.geom.spv

%GlslangValidator% -V 10_shader.comp -o 10_shader.comp.spv

%GlslangValidator% -V 11_model.vert -o 11_model.vert.spv
%GlslangValidator% -V 11_model.frag -o 11_model.frag.spv
%GlslangValidator% -V 11_normal.vert -o 11_normal.vert.spv
%GlslangValidator% -V 11_normal.frag -o 11_normal.frag.spv
%GlslangValidator% -V 11_normal.geom -o 11_normal.geom.spv

%GlslangValidator% -V 12_shader.vert -o 12_shader.vert.spv
%GlslangValidator% -V 12_shader.frag -o 12_shader.frag.spv

%GlslangValidator% -V 13_shader.vert -o 13_shader.vert.spv
%GlslangValidator% -V 13_shader.frag -o 13_shader.frag.spv

%GlslangValidator% -V 14_shader.vert -o 14_shader.vert.spv
%GlslangValidator% -V 14_shader.frag -o 14_shader.frag.spv

pause
