#!/usr/bin/ruby
#


####
#
# Rake Helpers
#
# (c) 2011, Bjoern Rennhak <bjoern AT rennhak DOT com>
#
############


public

  # = The function colorize takes a message and wraps it into standard color commands such as for bash.
  # @param color String, of the colorname in plain english. e.g. "LightGray", "Gray", "Red", "BrightRed"
  # @param message String, of the message which should be wrapped
  # @returns String, colorized message string
  # WARNING: Might not work for your terminal
  # FIXME: Implement bold behavior
  # FIXME: This method is currently b0rked
  def colorize color, message # {{{

    # Black       0;30     Dark Gray     1;30
    # Blue        0;34     Light Blue    1;34
    # Green       0;32     Light Green   1;32
    # Cyan        0;36     Light Cyan    1;36
    # Red         0;31     Light Red     1;31
    # Purple      0;35     Light Purple  1;35
    # Brown       0;33     Yellow        1;33
    # Light Gray  0;37     White         1;37

    colors  = { 
      "Gray"        => "\e[1;30m",
      "LightGray"   => "\e[0;37m",
      "Cyan"        => "\e[0;36m",
      "LightCyan"   => "\e[1;36m",
      "Blue"        => "\e[0;34m",
      "LightBlue"   => "\e[1;34m",
      "Green"       => "\e[0;32m",
      "LightGreen"  => "\e[1;32m",
      "Red"         => "\e[0;31m",
      "LightRed"    => "\e[1;31m",
      "Purple"      => "\e[0;35m",
      "LightPurple" => "\e[1;35m",
      "Brown"       => "\e[0;33m",
      "Yellow"      => "\e[1;33m",
      "White"       => "\e[1;37m"
    }
    nocolor    = "\e[0m"

    colors[ color ] + message + nocolor
  end # of def colorize }}}


  # = The function message will take a message as argument as well as a level (e.g. "info", "ok", "error", "question", "debug") which then would print 
  #   ( "(--) msg..", "(II) msg..", "(EE) msg..", "(??) msg..")
  # @param level Ruby symbol, can either be :info, :success, :error or :question
  # @param msg String, which represents the message you want to send to stdout (info, ok, question) stderr (error)
  # Helpers: colorize
  def message level, msg # {{{

    symbols = {
      :info      => [ "(--)", "Brown"       ],
      :success   => [ "(II)", "LightGreen"  ],
      :error     => [ "(EE)", "LightRed"    ],
      :question  => [ "(??)", "LightCyan"   ],
      :debug     => [ "(++)", "LightBlue"   ],
      :warning   => [ "(WW)", "Yellow"      ]
    }

    raise ArugmentError, "Can't find the corresponding symbol for this message level (#{level.to_s}) - is the spelling wrong?" unless( symbols.key?( level )  )

    if( level == :error )
      STDERR.puts colorize( symbols[ level.to_sym ].last, "#{symbols[ level.to_sym ].first.to_s} #{msg.to_s}" )
    else
      STDOUT.puts colorize( symbols[ level.to_sym ].last, "#{symbols[ level.to_sym ].first.to_s} #{msg.to_s}" )
    end

  end # of def message }}}


