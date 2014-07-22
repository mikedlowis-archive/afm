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
  env['CPPPATH'] += Dir['modules/data-structures/source/**/']
  #env['CFLAGS'] += ['-Wall']
  env['CFLAGS'] += ['-Werror', '-pedantic', '--std=c99']
  env['LIBS'] = ['ncurses']
end
at_exit { Env.process }

#------------------------------------------------------------------------------
# Rake Tasks
#------------------------------------------------------------------------------
task :default => [:build]

desc "Build the AFM release binary"
task :build => ['modules/data-structures/source'] do
  Env.Program('build/afm', Dir['source/**/*.c', 'modules/data-structures/source/**/*.c'])
end

desc "Remove all generate artifacts and directories"
task(:clean) { Rscons.clean }

desc "Remove all generated artifacts and directories as well as any git submodules"
task :clobber => [:clean] do
    sh 'git submodule deinit -f .'
end
