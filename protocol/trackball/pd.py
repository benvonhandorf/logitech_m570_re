import sigrokdecode as srd
import math

class Decoder(srd.Decoder):
  api_version = 2
  id = 'm570'
  name = 'm570 protocol'
  longname = 'Logitech m570 trackball SPI protocol'
  desc = 'Logitech m570 trackball SPI protocol'
  license = 'gplv2+'
  inputs = ['spi']
  outputs = ['x', 'y']
  channels = ()
  optional_channels = (
    {'id': 'cs', 'name': 'CS#', 'desc': 'Chip-select'},
  )

  annotations = (
    ('x', 'X Coordinate Moves'),
    ('y', 'Y Coordinate Moves'),
    ('cfg', 'Configuration'),
    ('cmd', 'Commands'),
    ('state', 'State'),
  )

  def start(self):
    self.out_ann = self.register(srd.OUTPUT_ANN)
    self.previous_mosi = 0x00
    self.initialized = False

  def decode(self, ss, es, data):
    ptype, mosi, miso = data

    if ptype == 'CS-CHANGE' and miso == 1:
      # Chip Select is de-asserted.  Clear state and flush anything existing
      if self.previous_mosi != 0x00:
        self.put(ss, es, self.out_ann, [2, ['{:02X} = '.format(self.previous_mosi)]])
        self.previous_mosi = 0x00
        return

    elif ptype == 'DATA':
      if self.initialized:
        if self.previous_mosi == 0x84:
          yDelta = miso
          
          if yDelta != 0:
            self.put(ss, es, self.out_ann, [1, ['Y:{:02X}'.format(yDelta), str(yDelta)]])

          self.previous_mosi = mosi
        elif self.previous_mosi == 0x85:
          xDelta = miso
          
          if xDelta != 0:
            self.put(ss, es, self.out_ann, [0, ['X:{:02X}'.format(xDelta), str(xDelta)]])

          self.previous_mosi = mosi
        elif mosi == 0x80:
          self.previous_mosi = 0x00
        elif self.previous_mosi & 0x80:
          self.put(ss, es, self.out_ann, [3, ['{:02X} = {:02X}'.format(self.previous_mosi, mosi)]])
          self.previous_mosi = 0x00
        else:
          self.previous_mosi = mosi
      else:
        if self.previous_mosi != 0x00:
          self.put(ss, es, self.out_ann, [2, ['{:02X} = {:02X}'.format(self.previous_mosi, mosi)]])

          if self.previous_mosi == 0x02 and mosi == 0x81:
            self.initialized = True
            self.put(ss,es, self.out_ann, [4, ['Initialized']])

          self.previous_mosi = 0x00

        else:
          self.previous_mosi = mosi


