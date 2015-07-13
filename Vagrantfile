# -*- mode: ruby -*-
# vi: set ft=ruby :

# Vagrantfile API/syntax version. Don't touch unless you know what you're doing!
VAGRANTFILE_API_VERSION = '2'

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|
  script = <<SCRIPT
  apt-get -y update
  apt-get -y install gcc rake bison git make libnl-dev
  if [ ! -d /usr/local/src/mruby ]; then
    git clone https://github.com/mruby/mruby/ /usr/local/src/mruby
  fi
  cp -a /vagrant/.vagrant_default.gembox /usr/local/src/mruby/mrbgems/default.gembox
SCRIPT

  config.vm.define :trusty do |c|
    c.vm.hostname  = 'mruby-ipvs-trusty'
    c.vm.hostname += "-#{ENV['WERCKER_BUILD_ID']}" if ENV['WERCKER_BUILD_ID']
    c.vm.provider :virtualbox do |_, override|
      override.vm.box = 'ubuntu/trusty64'
      config.vm.provision 'shell', inline: script
    end

    c.vm.provider :digital_ocean do |provider, override|
      override.ssh.private_key_path = '~/.ssh/id_rsa.vagrant'
      override.vm.box = 'digital_ocean'
      override.vm.box_url = 'https://github.com/smdahlen/vagrant-digitalocean/raw/master/box/digital_ocean.box'

      provider.ssh_key_name = ENV['WERCKER'] ? 'vagrant/wercker/mruby-ipvs' : 'Vagrant'
      provider.token = ENV['DIGITALOCEAN_TOKEN']
      provider.image = 'ubuntu-14-04-x64' # ubuntu
      provider.region = 'nyc3'
      provider.size = '512mb'
    end
  end
end
