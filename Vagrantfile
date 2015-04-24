# -*- mode: ruby -*-
# vi: set ft=ruby :

# Vagrantfile API/syntax version. Don't touch unless you know what you're doing!
VAGRANTFILE_API_VERSION = '2'

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|
  script = <<SCRIPT
    apt-get -y update
    apt-get -y install rake bison git make libnl-dev
    if [ ! -d /usr/local/src/mruby ]; then
      git clone https://github.com/mruby/mruby/ /usr/local/src/mruby
    fi
    cd /usr/local/src/mruby && rake test
SCRIPT

  config.vm.box = 'ubuntu/trusty64'
  config.vm.provision 'shell', inline: script
end
