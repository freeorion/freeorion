#!/usr/bin/env bash
#
# Uninstalling Mono on macOS.
#

current_userid=$(id -u)
if [ $current_userid -ne 0 ]; then
    echo "$(basename "$0") uninstallation script requires superuser privileges to run" >&2
    exit 1
fi

mono_xamarin_pkg_name_suffix="com.xamarin.mono"
mono_install_root="/Library/Frameworks/Mono.framework"
mono_path_file="/etc/paths.d/mono-commands"

remove_mono_pkgs(){
    installed_pkgs=($(pkgutil --pkgs | grep $mono_xamarin_pkg_name_suffix))
    
    for i in "${installed_pkgs[@]}"
    do
        echo "Removing mono component - \"$i\"" >&2
        pkgutil --force --forget "$i"
    done
}

remove_mono_pkgs
[ "$?" -ne 0 ] && echo "Failed to remove mono packages." >&2 && exit 1

echo "Deleting install root - $mono_install_root" >&2
rm -rf "$mono_install_root"
rm -f "$mono_path_file"

echo "mono removal succeeded." >&2
exit 0
