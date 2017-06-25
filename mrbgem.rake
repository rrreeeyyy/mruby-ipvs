MRuby::Gem::Specification.new('mruby-ipvs') do |spec|
  spec.license = 'GNU General Public License'
  spec.author  = 'YOSHIKAWA Ryota'
  spec.add_test_dependency 'mruby-struct'
  spec.cc.include_paths << "#{spec.dir}/ipvsadm/libipvs"
  spec.linker.libraries << ['nl']
  spec.objs << (Dir.glob("#{spec.dir}/src/*.c") +
                Dir.glob("#{spec.dir}/ipvsadm/libipvs/*.c")).map do |f|
    f.relative_path_from(spec.dir).pathmap("#{build_dir}/%X.o")
  end
end
