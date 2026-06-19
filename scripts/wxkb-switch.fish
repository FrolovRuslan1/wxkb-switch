# This is fish wxkb-switch completion

complete -c wxkb-switch -f -r
complete -c wxkb-switch -s h -l help    -d 'Display this help message.'
complete -c wxkb-switch -s n -l next    -d 'Set next xkb layout.'
complete -c wxkb-switch -s p -l prev    -d 'Set previous xkb layout.'
complete -c wxkb-switch -s l -l list    -d 'List available layouts.'
complete -c wxkb-switch -s v -l version -d 'Print program version.'
complete -c wxkb-switch -s d -l debug   -d 'Enable debug mode.'