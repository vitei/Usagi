# Usagi Engine, Copyright © Vitei, Inc. 2013
require 'nokogiri'
require 'Engine/Framework/FrameworkComponents.pb.rb'
require 'Engine/Graphics/Lights/LightSpec.pb.rb'

require_relative 'matrix_util.rb'

module LightExtractor

  ##################
  # classes        #
  ##################
  class LightNode
    attr_accessor :name, :type, :position, :direction
    attr_accessor :has_shadow, :ambient, :diffuse
    attr_accessor :specular, :inner_angle, :outer_angle
    attr_accessor :atten_enabled, :atten_start, :atten_end

    def initialize(name)
      @name = name
      @type = Usg::LightKind::DIRECTIONAL
      @position = {}
      @direction = {}
      @has_shadow = false
      @ambient = {}
      @diffuse = {}
      @specular = {}
      @inner_angle = 0.0
      @outer_angle = 0.0
      @atten_enabled = false
      @atten_start = 0.0
      @atten_end = 10000.0
    end

    def to_object

      base = { 'kind' => @type, 'ambient' => @ambient,
        'diffuse' => @diffuse, 'specular' => @specular, 'bShadow' => @has_shadow }
      atten = { 'bEnabled' => @atten_enabled, 'fNear' => @atten_start, 'fEnd' => @atten_end }
      spot = { 'fInnerAngle' => @inner_angle, 'fOuterAngle' => @outer_angle }
      spec = {'direction' => @direction, 'base' => base, 'atten' => atten, 'spot' => spot}
      # Projection lights not available in models
      object = {'Identifier'=>{'name' => @name}, 'TransformComponent'=>{'position' => @position},
        'LightComponent' => {'spec' => spec} 
      }

      return object
    end
  end

  ##################
  # constants      #
  ##################
  VECTOR_FIELDS = ['position', 'direction']
  COLOR_FIELDS = ['ambient', 'diffuse', 'specular']
  VEC_COMPONENTS = ['x', 'y', 'x']
  CLR_COMPONENTS = ['m_fR', 'm_fG', 'm_fB']

  def self.extract(model_filename, rootBone)
    doc = Nokogiri::XML(File.read(model_filename))

    queries = {:skeleton => 'hierarchy', :light_array => 'light_array', :lights => 'light_array > light', :name => 'name',
      :parent => 'parent_name', :type => 'type', :has_shadow => 'has_shadow',
      :inner_angle => 'inner_angle', :outer_angle => 'outer_angle', 
      :atten_enabled => 'atten_enabled', :atten_start => 'atten_start', :atten_end => 'atten_end'
      }

    lighting = doc.at_css(queries[:skeleton])


    lighting.css(queries[:lights]).sort.each do |b|

      lightName = b[queries[:name]]
      parentBoneName = b[queries[:parent]]

      if parentBoneName.length > 1
        node = LightNode.new(lightName)
        parentNode = rootBone.find(parentBoneName, 0)

        if parentNode[0] == nil
          raise "couldn't find parent bone '#{parentBoneName}' in input file '#{model_filename}'"
          STDERR.puts "Not found"
        end

        parentNode[0].add_light(node)
      end

      LightExtractor::VECTOR_FIELDS.map{|f| f}.each do |field_name|
        values = b[field_name].split(' ').slice(0, 3)

        LightExtractor::VEC_COMPONENTS.each_with_index do |c, index|
          node.instance_eval("#{field_name}['#{c}'] = #{values[index].to_f}")
        end
      end

      LightExtractor::COLOR_FIELDS.map{|f| f}.each do |field_name|
        values = b[field_name].split(' ').slice(0, 3)

        LightExtractor::CLR_COMPONENTS.each_with_index do |c, index|
          node.instance_eval("#{field_name}['#{c}'] = #{values[index].to_f}")
        end        
      end

      # Dangling values
      node.type = Usg::LightKind::const_get(b[queries[:type]].upcase)
      STDERR.puts "Type: " + b[queries[:type]].upcase
      node.has_shadow = b[queries[:has_shadow]].upcase == 'TRUE' ? true : false
      node.inner_angle = b[queries[:inner_angle]].to_f
      node.outer_angle = b[queries[:outer_angle]].to_f
      node.atten_enabled = b[queries[:atten_enabled]].upcase == 'TRUE' ? true : false
      node.atten_start = b[queries[:atten_start]].to_f
      node.atten_end = b[queries[:atten_end]].to_f

    end

    return rootBone.to_object
  end
end
