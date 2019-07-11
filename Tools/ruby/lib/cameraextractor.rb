# Usagi Engine, Copyright © Vitei, Inc. 2013
require 'nokogiri'
require 'Engine/Framework/FrameworkComponents.pb.rb'
require 'Engine/Scene/Common/SceneComponents.pb.rb'

require_relative 'matrix_util.rb'

module CameraExtractor

  ##################
  # classes        #
  ##################
  class CameraNode
    attr_accessor :name, :position, :rotate
    attr_accessor :fov, :near, :far

    def initialize(name)
      @name = name
      @position = {}
      @rotate = {}
      @fov = 50.0
      @near = 1.0
      @far = 1000.0
    end

    def to_object
      rotationMatrix = Matrix.rotation_matrix4x4(@rotate['x'], @rotate['y'], @rotate['z'])
      rotationMatrix.set_translation(@position['x'], @position['y'], @position['z'])
      object = {'Identifier'=>{'name' => @name}, 'TransformComponent'=>{'position' => @position,
        'rotation' => rotationMatrix.rotation_matrix_to_quaternion()},
        'MatrixComponent'=>{}, 'CameraComponent' => {'fFOV' => @fov, 'fNearPlaneDist' => @near,
          'fFarPlaneDist'=>@far} 
      }

      return object
    end
  end

  ##################
  # constants      #
  ##################
  VECTOR_FIELDS = ['position', 'rotate']
  VEC_COMPONENTS = ['x', 'y', 'z']

  def self.extract(model_filename, rootBone)
    doc = Nokogiri::XML(File.read(model_filename))

    queries = {:skeleton => 'hierarchy', :camera_array => 'camera_array', :cameras => 'camera_array > camera', :name => 'name',
      :parent => 'parent_name', :fov => 'fov', :near => 'near',
      :far => 'far'
      }

    cameras = doc.at_css(queries[:skeleton])


    cameras.css(queries[:cameras]).sort.each do |b|

      cameraName = b[queries[:name]]
      parentBoneName = b[queries[:parent]]

      if parentBoneName.length > 1
        node = CameraNode.new(cameraName)
        parentNode = rootBone.find(parentBoneName, 0)

        if parentNode[0] == nil
          raise "couldn't find parent bone '#{parentBoneName}' in input file '#{model_filename}'"
          STDERR.puts "Not found"
        end

        parentNode[0].add_camera(node)
      end

      CameraExtractor::VECTOR_FIELDS.map{|f| f}.each do |field_name|
        values = b[field_name].split(' ').slice(0, 3)

        CameraExtractor::VEC_COMPONENTS.each_with_index do |c, index|
          node.instance_eval("#{field_name}['#{c}'] = #{values[index].to_f}")
        end
      end

      node.fov = b[queries[:fov]].to_f
      node.near = b[queries[:near]].to_f
      node.far = b[queries[:far]].to_f


    end

    return rootBone.to_object
  end
end
