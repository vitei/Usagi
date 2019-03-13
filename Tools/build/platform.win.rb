# Usagi Engine, Copyright (c) Vitei, Inc. 2013
require 'pathname'

require_relative 'platform.rb'

class PlatformWin < Platform

  #####################################################################
  # Windows-specific Paths and Tools                                  #
  #####################################################################

  def underscore_dirs_whitelist
    ['_win', '_ogl', '_fragment', '_xaudio2']
  end
end
