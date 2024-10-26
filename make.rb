#!/usr/bin/env ruby

require 'fileutils'

# Configuration
prog = 'rogue'
cflags = '-O2 -Wall -Wextra -Wpedantic -fdiagnostics-color=always -Wno-nullability-extension'
cflags += ' -I/usr/local/include -I/usr/X11R6/include'
ldflags = '-L/usr/local/lib'
ldadd = '-lglfw -lstdc++ -lpthread -lvulkan -lm'

srcdir = File.join(Dir.pwd, 'src')
objdir = File.join(Dir.pwd, 'obj')

# List of source files
srcs = Dir.glob(File.join(srcdir, '*.c'))
objs = srcs.map { |src| File.join(objdir, File.basename(src, '.c') + '.o') }

# Ensure the object directory exists
FileUtils.mkdir_p(objdir)

# Clean up build artifacts
if ARGV.include?('clean')
  FileUtils.rm_f([prog, 'obj/vma.o', 'shaders/vert.spv', 'shaders/frag.spv'] + objs)
  exit
end

# Compile shader files
def compile_shader(input, output)
  cmd = "glslangValidator -V #{input} -o #{output}"
  puts cmd
  system cmd
end

# Compile C source files to object files
def compile_source(src, obj, cflags)
  cmd = "cc #{cflags} -Wno-unused-parameter -c #{src} -o #{obj}"
  puts cmd
  system cmd
end

# Compile the program
def link_program(cflags, prog, objs, ldflags, ldadd)
  cmd = "cc #{cflags} #{objs.join(' ')} -o #{prog} #{ldflags} #{ldadd}"
  puts cmd
  system cmd
end

# Compile the shaders
compile_shader('shaders/shader.vert', 'shaders/vert.spv')
compile_shader('shaders/shader.frag', 'shaders/frag.spv')

# Compile the additional C++ file
system("c++ -O0 -Wno-nullability-completeness -c extern/vma.cpp -o obj/vma.o -I/usr/local/include")

# Compile the C source files
srcs.each_with_index do |src, index|
  compile_source(src, objs[index], cflags)
end

# Link the program
link_program(cflags, prog, Dir.glob(File.join(objdir, '*.o')), ldflags, ldadd)
