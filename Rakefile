#------------------------------------------------------------------------------
# Verify and Load Dependencies
#------------------------------------------------------------------------------
require "bundler"
begin
  Bundler.setup(:default, :development)
rescue Bundler::BundlerError => e
  raise LoadError.new("Unable to Bundler.setup(): You probably need to run `bundle install`: #{e.message}")
end
require 'rscons'

#------------------------------------------------------------------------------
# Checkout Git Submodules
#------------------------------------------------------------------------------
if Dir['modules/**/.git'].length == 0
  sh 'git submodule update --init'
end

#------------------------------------------------------------------------------
# Build Environment
#------------------------------------------------------------------------------
Env = Rscons::Environment.new do |env|
  env.build_dir('source','build/obj/source')
  env.build_dir('modules','build/obj/modules')
  env['LIBS'] = ['ncurses', 'cds']
  env['LIBPATH'] += ['./build']
  env['CPPPATH'] += Dir['modules/data-structures/source/**/']
  env['CFLAGS'] += ['-Wall', '-Wextra',
    #'-Werror', #commented out until mike fixes his shit.
    '-pedantic', '--std=c99']

  # Platform-Specific Defines and Options
  # -------------------------------------
  if RUBY_PLATFORM =~ /linux/
    env['CFLAGS'] += [
        '-D_GNU_SOURCE',
        '-D_XOPEN_SOURCE=700'
    ]
  elsif RUBY_PLATFORM =~ /mingw/
    env['CFLAGS'] -= ['--std=c99']
    env['CFLAGS'] += ['--std=gnu99']
  elsif RUBY_PLATFORM =~ /darwin/
    env['CFLAGS'] += [
      '-D_DARWIN_C_SOURCE',
      '-D__DARWIN_C_LEVEL=199506L'
    ]
  end
end
at_exit { Env.process }

#------------------------------------------------------------------------------
# Rake Tasks
#------------------------------------------------------------------------------
task :default => [:build]

desc "Build the AFM release binary"
task :build => ['modules/data-structures/source', :libcds] do
  Env.Program('build/afm', Dir['source/**/*.c'])
end

desc "Build the CDS static library"
task :libcds do
  Env.Library('build/libcds.a', Dir['modules/data-structures/source/**/*.c'])
end

desc "Remove all generate artifacts and directories"
task(:clean) { Rscons.clean }

desc "Remove all generated artifacts and directories as well as any git submodules"
task :clobber => [:clean] do
  sh 'git submodule deinit -f .'
end

task :valgrind => [:build] do
  sh "valgrind --leak-check=full ./build/afm 2> ./build/valgrind.txt"
end

task :splint do
  include_dirs = Dir['source/**/','modules/**/source/**/'].map{|e| "-I#{e}" }
  sources = Dir['source/**/*.c', 'modules/data-structures/source/**/*.c']
  cmd = ['splint', '+posixlib'] + include_dirs + sources
  sh *cmd
end
