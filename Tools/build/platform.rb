# Usagi Engine, Copyright (c) Vitei, Inc. 2013
require 'pathname'

class Platform
  def initialize()
  end

  def underscore_dirs_whitelist
    []
  end

  def cc
    ''
  end

  def cxx
    ''
  end

  def cm
    ''
  end

  def cmm
    ''
  end

  def ar
    ''
  end

  def ld
    ''
  end

  def cppflags
    []
  end

  def cmflags
    []
  end

  def cflags
    []
  end

  def cxxflags
    []
  end

  def pchflags
    []
  end

  def create_pch_flag
    ''
  end

  def use_pch_flag
    ''
  end

  def c_cpp_dep_flags
    []
  end

  def arflags
    []
  end

  def ldflags
    []
  end

  def system_libs
    []
  end

  def layout_converter_flags
    ''
  end
end
