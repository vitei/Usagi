# Usagi Engine, Copyright © Vitei, Inc. 2013
require 'matrix'

class Matrix
  def set_translation(x, y, z)
    raise "Error! This method can only be used on 4x4 matrices!" if row_size() != 4 || column_size() != 4

    @rows[3][0] = x
    @rows[3][1] = y
    @rows[3][2] = z
    @rows[3][3] = 1.0
  end

  def rotation_matrix_to_quaternion()
    raise "Error! This method only works on matrices of size 3x3 or larger!" if row_size() < 3 || column_size() < 3

    quaternion = {'x' => 0.0, 'y' => 0.0, 'z' => 0.0, 'w' => 0.0}
    trace = trace()

    if trace > 0.0
      quaternion['x'] = (@rows[1][2] - @rows[2][1]) / (2.0 * Math.sqrt(trace))
      quaternion['y'] = (@rows[2][0] - @rows[0][2]) / (2.0 * Math.sqrt(trace))
      quaternion['z'] = (@rows[0][1] - @rows[1][0]) / (2.0 * Math.sqrt(trace))
      quaternion['w'] = Math.sqrt(trace) / 2.0

      return quaternion
    end

    maxI = 0
    maxDiag = @rows[0][0]

    for i in 1..2 do
      if @rows[i][i] > maxDiag
        maxI = i
        maxDiag = @rows[i][i]
      end
    end

    case maxI
    when 0
      s = 2.0 * Math.sqrt(1.0 + @rows[0][0] - @rows[1][1] - @rows[2][2])
			quaternion['x'] = 0.25 * s
			quaternion['y'] = ( @rows[0][1] + @rows[1][0] ) / s
			quaternion['z'] = ( @rows[0][2] + @rows[2][0] ) / s
			quaternion['w'] = ( @rows[1][2] - @rows[2][1] ) / s
    when 1
			s = 2.0 * Math.sqrt(1.0 + @rows[1][1] - @rows[0][0] - @rows[2][2])
			quaternion['x'] = ( @rows[0][1] + @rows[1][0] ) / s
			quaternion['y'] = 0.25 * s
			quaternion['z'] = ( @rows[1][2] + @rows[2][1] ) / s
			quaternion['w'] = ( @rows[2][0] - @rows[0][2] ) / s
    when 2
			s = 2.0 * Math.sqrt(1.0 + @rows[2][2] - @rows[0][0] - @rows[1][1])
			quaternion['x'] = ( @rows[0][2] + @rows[2][0] ) / s
			quaternion['y'] = ( @rows[1][2] + @rows[2][1] ) / s
			quaternion['z'] = 0.25 * s
			quaternion['w'] = ( @rows[0][1] - @rows[1][0] ) / s
    end

    return quaternion
  end

  def self.rotation_matrix4x4(x, y, z)
    cx = Math.cos(x)
    sx = Math.sin(x)
    cy = Math.cos(y)
    sy = Math.sin(y)
    cz = Math.cos(z)
    sz = Math.sin(z)

    row1 = Array.new(4, 0.0)
    row2 = Array.new(4, 0.0)
    row3 = Array.new(4, 0.0)
    row4 = Array.new(4, 0.0)

    row1[0] = cy * cz
    row1[1] = cy * sz
    row1[2] = -sy
    row1[3] = 0.0

    row2[0] = (sx * sy * cz) - (cx * sz)
    row2[1] = (sx * sy * sz) + (cx * cz)
    row2[2] = sx * cy
    row2[3] = 0.0

    row3[0] = (cx * sy * cz) + (sx * sz)
    row3[1] = (cx * sy * sz) - (sx * cz)
    row3[2] = cx * cy
    row3[3] = 0.0

    row4[3] = 1.0

    return self.rows([row1, row2, row3, row4])
  end
end
