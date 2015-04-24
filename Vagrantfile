# -*- mode: ruby -*-
# vi: set ft=ruby :

# Vagrantfile API/syntax version. Don't touch unless you know what you're doing!
VAGRANTFILE_API_VERSION = '2'

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|
  script = <<SCRIPT
    apt-get -y update
    apt-get -y install rake
    apt-get -y install bison
    apt-get -y install git
    apt-get -y install libnl-dev
    apt-get -y install make
    git clone https://github.com/mruby/mruby/ /usr/local/src/mruby
SCRIPT

  config.vm.box = 'ubuntu/trusty64'
  config.vm.provision 'shell', inline: script
end
