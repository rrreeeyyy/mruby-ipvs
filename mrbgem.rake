MRuby::Gem::Specification.new('mruby-ipvs') do |spec|
  spec.license = 'GNU General Public License'
  spec.author  = 'YOSHIKAWA Ryota'
  spec.cc.include_paths << "#{spec.dir}/libipvs-2.6"
  spec.linker.libraries << ['nl']
  spec.objs << (Dir.glob("#{spec.dir}/src/*.c") +
                Dir.glob("#{spec.dir}/libipvs-2.6/*.c")).map { |f|
    f.relative_path_from(spec.dir).pathmap("#{build_dir}/%X.o")
  }
end
