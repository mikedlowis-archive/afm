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
# Build Environment
#------------------------------------------------------------------------------
Env = Rscons::Environment.new do |env|
  env['CFLAGS'] += ['-Wall', '-Werror']
  env['LIBS'] = ['ncurses']
end
at_exit { Env.process }

#------------------------------------------------------------------------------
# Rake Tasks
#------------------------------------------------------------------------------
#task :default

desc "Build the AFM release binary"
task :build do
  Env.Program('build/afm', Dir['source/**/*.c'])
end

desc "Remove all generate artifacts and directories"
task(:clean) { Rscons.clean }
