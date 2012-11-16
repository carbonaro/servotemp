servotemp
=========

Standard 9G servo + MLX90614 thermometer + Arduino Uno + Node.Js + Raspberry PI = autonomous floor &amp; ceiling IR temperature monitor

# Install on RPi

npm is not (fully?) running the binding.gyp in serialport (le sigh) -- 
the fix was simple on my end, cd into the node_modules/serialport and issue a quick node-gyp configure 
and a nice solid node-gyp build. Keep in mind you'll need a basic toolchain (etc), 
which can be provided with build-essential (at least).

# Credits

Thanks to Bildr.org! http://bildr.org/2011/02/mlx90614-arduino/
