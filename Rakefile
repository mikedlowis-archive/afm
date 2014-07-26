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
  env['LIBS'] = ['ncurses']
  env['CPPPATH'] += Dir['modules/data-structures/source/**/']
  #env['CFLAGS'] += ['-Wall']
  env['CFLAGS'] += ['-Werror', '-pedantic', '--std=c99']

  # Platform-specific Defines
  # -------------------------
  if RUBY_PLATFORM =~ /linux/
    env['CFLAGS'] += [
        '-D_GNU_SOURCE',
        '-D_XOPEN_SOURCE=700'
    ]
  elsif RUBY_PLATFORM =~ /cygwin/
    # TODO
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
task :build => ['modules/data-structures/source'] do
  Env.Program('build/afm', Dir['source/**/*.c', 'modules/data-structures/source/**/*.c'])
end

desc "Remove all generate artifacts and directories"
task(:clean) { Rscons.clean }

desc "Remove all generated artifacts and directories as well as any git submodules"
task :clobber => [:clean] do
    sh 'git submodule deinit -f .'
end
