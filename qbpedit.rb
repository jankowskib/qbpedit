#!/usr/bin/env ruby
#   Quick Build.prop Editor
#   qbpedit.rb
#
#   Copyright 2013-2015 Bartosz Jankowski
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
require 'optparse'
require 'inifile'


# Hack to treat every value as String
class IniFile::Parser
  def typecast( value )
    case value
    when %r/\Atrue\z/i;  true
    when %r/\Afalse\z/i; false
    when %r/\A\s*\z/i;   nil
    else
      unescape_value(value)
    end
  end

end

$options = {}

begin
  OptionParser.new do |opts|
    opts.banner = "Usage: qbpedit.rb [options]"
    opts.separator "Options:"
    opts.on("-m", "--model text", "Change model name") { |t| $options[:model] = t }
    opts.on("-p", "--producer text", "Change producer name") { |t| $options[:producer] = t }
    opts.on("-l", "--language text", "Change default language") { |t| $options[:lang] = t }
    opts.on("-z", "--timezone text", "Change default timezone (Continent/City)") { |t| $options[:zone] = t }
    opts.on("-v", "--version text", "Change version") { |t| $options[:version] = t }
    opts.on("-d", "--dev text", "Change build developer") { |t| $options[:dev] = t }
    opts.on_tail("-t", "--update-timestamp", "Update build time") { $options[:time] = true}
  end.parse!

  $options[:file] = "build.prop" unless $options[:file]
  begin
  f = File.read($options[:file])
  rescue => e
    puts "Failed to read file (#{options[:file]}) : #{e}"
    exit -1
  end
  ini = IniFile.new(:content => f)
  gms_model = $options[:model] ? $options[:model] : ini[:global]["ro.product.name"]
  gms_model = gms_model.gsub(/[\s\.\+]/,"_").downcase
  gms_model.chop! if gms_model && gms_model[-1] == "_"

  if $options[:model]
    ini[:global]["ro.product.model"] = $options[:model] if ini[:global]["ro.product.model"]
    ini[:global]["ro.product.name"] = gms_model if ini[:global]["ro.product.name"]
    ini[:global]["ro.product.device"] = gms_model if ini[:global]["ro.product.device"]
    ini[:global]["ro.build.product"] = gms_model if ini[:global]["ro.build.product"]
    ini[:global]["ro.product.usbfactory"] = gms_model if ini[:global]["ro.product.usbfactory"]
    ini[:global]["ro.udisk.lable"] = gms_model if ini[:global]["ro.udisk.lable"]
  end
  if $options[:producer]
    ini[:global]["ro.product.brand"] = $options[:producer] if ini[:global]["ro.product.brand"]
    ini[:global]["ro.product.manufacturer"] = $options[:producer] if ini[:global]["ro.product.manufacturer"]
  end
  if $options[:dev]
    ini[:global]["ro.build.user"] = $options[:dev] if ini[:global]["ro.build.user"]
    ini[:global]["ro.build.host"] = `uname --n`.chomp if ini[:global]["ro.build.host"]
  end
  if $options[:lang]
    ini[:global]["ro.product.locale.language"] = $options[:lang][/([a-z]+)/] if ini[:global]["ro.product.locale.language"]
    ini[:global]["persist.sys.language"] = $options[:lang][/([a-z]+)/]  if ini[:global]["persist.sys.language"]

    ini[:global]["ro.product.locale.region"] = $options[:lang][/([A-Z]+)/] if ini[:global]["ro.product.locale.region"]
    ini[:global]["persist.sys.country"] = $options[:lang][/([A-Z]+)/]  if ini[:global]["persist.sys.country"]
  end
  if $options[:time]
    time = Time.now
    ini[:global]["ro.build.date.utc"] = time.to_i
    ini[:global]["ro.build.date"] = time.strftime("%a %b %m %d %T %Z %Y")
  end
  if $options[:zone]
    ini[:global]["persist.sys.timezone"] = $options[:zone]
  end

  # update fingerprint
  ini[:global]["ro.build.fingerprint"] = "#{ini[:global]["ro.product.brand"]}/" <<
    gms_model << "/" << "#{gms_model}:" << ini[:global]["ro.build.version.release"] <<
    "/#{ini[:global]["ro.build.id"]}/#{ini[:global]["ro.build.version.incremental"]}:" <<
    "#{ini[:global]["ro.build.type"]}/#{ini[:global]["ro.build.tags"]}"
  # update description
  ini[:global]["ro.build.description"] = "#{gms_model}-#{ini[:global]["ro.build.type"]}" <<
    " #{ini[:global]["ro.build.id"]} #{ini[:global]["ro.build.version.incremental"]} " <<
    ini[:global]["ro.build.tags"]
  # update display id
  ini[:global]["ro.build.display.id"] << ini[:global]["ro.product.name"] <<
    "-#{$options[:version]}" if $options[:version]
  ini[:global]["ro.custom.build.version"] = ini[:global]["ro.build.display.id"] if $options[:version]


  File.open($options[:file], "w") do |w|
    # make the file as much unmodified as possible
    f.each_line do |l|
      unless l.chomp.empty? || l[0] == '#'
        prop, val, val = l.partition("=")
        l = "#{prop}=#{ini[:global][prop]}\n" if val != ini[:global][prop]
      end
      w << l
    end
  end

end
