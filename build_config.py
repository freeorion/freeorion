# -*- Python -*-

freeorion_version = "v0.3.1-RC5"

gigi_version = '0.6.0'

ft_pkgconfig_version = '9.0.0'
ft_version = '2.1.2'
ft_win32_lib_name = 'freetype214MT'

devil_version_string = '1.6.1'
devil_version = ''.join(devil_version_string.split('.'))

sdl_version = '1.2.7'

boost_version_string = '1.33.1'
def BoostStringToNumber(version_string):
    pieces = version_string.split('.')
    return str(int(pieces[0]) * 100000 + int(pieces[1]) * 100 + int(pieces[2]))
boost_version = BoostStringToNumber(boost_version_string)

log4cpp_version = '0.3.4b'

fmod_version = '3.75'
fmod_win32_lib_name = 'fmodvc'

graphviz_pkgconfig_version = '2.8'
