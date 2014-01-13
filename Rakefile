#!/usr/bin/env ruby


desc "Generate doxygen documentation" # {{{
task :doxygen do |d|
  sh "doxygen doxygen"
end # }}}

desc "Show current version of project" # {{{
task :version do |v|
  puts "Current GIT version is:"
  puts `git describe --tags`
end # }}}

desc "Generate proper README file from templates" # {{{
task :readme do |t|
  source    = "README.md.template"
  target    = "README.md"

  content      = File.readlines( source ).collect!{ |line| line.rstrip }
  version   = `git describe --tags`.strip

  content[ content.index( "$Version$" ) ] = "Version " + version if( content.include?( "$Version$" ) )
  File.write( target, content.join("\n") )
  puts "(II) #{target.to_s} generated from #{source.to_s}"
end # }}}


# vim:ts=2:tw=100:wm=100:syntax=ruby
