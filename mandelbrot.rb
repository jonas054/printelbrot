# frozen_string_literal: true

require 'gosu'
require 'inline'

NR_OF_THREADS = 10
SCALE = 1

# Mandelbrot set viewer
class Mandelbrot < Gosu::Window
  def initialize
    super((1920.0 / SCALE).round, (1080.0 / SCALE).round, fullscreen: true)
    @x_offset = -0.649985 # -0.2349932
    @y_offset = 0.476856 # 0.8281560
    @max = 100
    @size = 8
    @hue_offset = 0
  end

  def update
    start = Time.now
    @result = Array.new(height) { [nil] * width }
    threads = NR_OF_THREADS.times.map do |ix|
      Thread.new(ix) { |i| executor(i, @size, @x_offset, @y_offset, @max) }
    end
    threads.each(&:join)
    @size *= 0.8
    @max *= 1.05
    p update: Time.now - start
  end

  def draw
    start = Time.now
    @hue_offset += 1
    puts "X:#{@x_offset} Y:#{@y_offset} S:#{@size} M:#{@max}"
    (0...height).each do |y|
      (0...width).each do |x|
        value = @result[y][x]
        color = if value >= @max - 1
                  Gosu::Color::BLACK
                else
                  Gosu::Color.from_hsv((@hue_offset + 360 * value / @max) % 360,
                                       1.0, 0.5 - 1.0 * value / @max / 2)
                end
        draw_rect(x, y, 1, 1, color)
      end
    end
    p draw: Time.now - start
  end

  private

  def executor(index, size, x_offset, y_offset, max)
    height.times do |row|
      y = coordinate(0.6, row, height, y_offset, size)
      col = index
      while col < width
        x = coordinate(1.0, col, width, -x_offset, size)
        @result[row][col] = iterations(x, y, max) - 1
        col += NR_OF_THREADS
      end
    end
  end

  def coordinate(scale, pos, full, offset, size)
    size * scale * (pos - full / 2.0) / full - offset
  end

  inline do |builder|
    builder.c <<~CODE
      long iterations(double x, double y, int max) {
        int count = 0;
        double zr = 0, zi = 0, zr2 = 0, zi2 = 0;
        for (; zr2 + zi2 < 4.0 && count < max; ++count) {
          zi = 2 * zr * zi + y;
          zr = zr2 - zi2 + x;
          zr2 = zr * zr;
          zi2 = zi * zi;
        }
        return count;
      }
    CODE
  end
end

Mandelbrot.new.show
# p m.iterations(0.1080729, -0.6386042, 100)
