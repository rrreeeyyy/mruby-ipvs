MRuby::Gem::Specification.new('mruby-ipvs') do |spec|
  spec.license = 'GNU General Public License'
  spec.author  = 'YOSHIKAWA Ryota'
  spec.cc.include_paths << "#{spec.dir}/ipvsadm/libipvs"
  spec.linker.libraries << ['nl']
  spec.objs << (Dir.glob("#{spec.dir}/src/*.c") +
                Dir.glob("#{spec.dir}/ipvsadm/libipvs/*.c")).map do |f|
    f.relative_path_from(spec.dir).pathmap("#{build_dir}/%X.o")
  end
  spec.add_test_dependency 'mruby-require'
  spec.add_test_dependency 'mruby-array-ext', :core => 'mruby-array-ext'
  spec.add_test_dependency 'mruby-hash-ext', :core => 'mruby-hash-ext'
  spec.add_test_dependency 'mruby-print', :core => 'mruby-print'
  spec.add_test_dependency('mruby-io', :github => 'iij/mruby-io')
end
