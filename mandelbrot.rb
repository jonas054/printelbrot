# frozen_string_literal: true

require 'gosu'
require 'inline'
require 'rainbow'

NR_OF_THREADS = 5
SCALE = 1
UPDATES_BETWEEN_CALCULATIONS = 8
ZOOM_FACTOR = 0.6

# Mandelbrot set viewer
class Mandelbrot < Gosu::Window
  def initialize
    super((1920.0 / SCALE).round, (1080.0 / SCALE).round, fullscreen: true)
    @x_offset = -0.1630677793 # -0.2349932
    @y_offset = 1.0261700759 # 0.8281560
    @max = 100
    @size = 8
    @updates_left_before_calc = 0
    @hue_offset = 0
  end

  def update
    start = Time.now
    if @updates_left_before_calc < 2
      wait_for_calculation if @threads
      start_new_calculation
      @updates_left_before_calc = UPDATES_BETWEEN_CALCULATIONS
      puts Rainbow("calculate: #{Time.now - start}").green
    else
      @updates_left_before_calc -= 1
      zoom_in if @last_result
      puts Rainbow("zoom: #{Time.now - start}").yellow
    end
  end

  def draw
    return unless @result_to_draw

    start = Time.now
    @hue_offset += 1

    puts "X:#{@x_offset} Y:#{@y_offset} S:#{@size} M:#{@max.to_i}"
    @result_to_draw.each_with_index do |row, y|
      x = 0
      while x < width
        value = row[x]
        x1 = x + 1
        x1 += 1 while x1 < width && row[x1] == value
        draw_rect(x, y, x1 - x, 1, color_of(value))
        x = x1
      end
    end
    puts Rainbow("draw: #{Time.now - start}").cyan
  end

  private

  def wait_for_calculation
    @threads.each(&:join)
    @result_to_draw = @last_result = @current_result
    @size *= ZOOM_FACTOR
    @max *= 1.2
  end

  def start_new_calculation
    @current_result = Array.new(height) { [nil] * width }
    @threads = NR_OF_THREADS.times.map do |ix|
      Thread.new(ix) { |i| executor(i, @size, @x_offset, @y_offset, @max) }
    end
  end

  def zoom_in
    zoom =
      ZOOM_FACTOR**(1 - @updates_left_before_calc.to_f / UPDATES_BETWEEN_CALCULATIONS)
    margin = (1 - zoom) / 2
    y_offset = height * margin
    x_offset = width * margin
    @result_to_draw = Array.new(height) { [nil] * width }
    (0...height).each do |y|
      (0...width).each do |x|
        @result_to_draw[y][x] = @last_result[y_offset + y * zoom][x_offset + x * zoom]
      end
    end
  end

  def color_of(value)
    if value >= @max
      Gosu::Color::BLACK
    else
      Gosu::Color.from_hsv((@hue_offset + 360 * value / @max) % 360,
                           1.0, 0.5 - 1.0 * value / @max / 2)
    end
  end

  def executor(index, size, x_offset, y_offset, max)
    height.times do |row|
      y = coordinate(0.6, row, height, y_offset, size)
      col = index
      while col < width
        x = coordinate(1.0, col, width, -x_offset, size)
        @current_result[row][col] = iterations(x, y, max)
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
