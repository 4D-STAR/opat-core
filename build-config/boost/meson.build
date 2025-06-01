boost_dep = dependency('boost', version: '>=1.8.0', required: false)

if not boost_dep.found()
    message('⚠️ System dependency "boost" not found; running install shell script…')

    install_boost_cmd = run_command('sh', meson.source_root() / 'build-config/boost/install.sh')
    if install_cmd.returncode() != 0
        error('‼️ Failed to run install.sh for system dependency "boost"')
    endif

    boost_dep = dependency('boost', version: '>=1.8.0', required: true)
    message('✅ System dependency "boost" installed sucsessfully!')
else
    message('✅ System dependency "boost" found!')
endif