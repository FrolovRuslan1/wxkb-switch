# This is fish wxkb_switch completion

complete -c wxkb_switch -f -r
complete -c wxkb_switch -s h -l help    -d 'Display this help message.'
complete -c wxkb_switch -s n -l next    -d 'Set next xkb layout.'
complete -c wxkb_switch -s p -l prev    -d 'Set previos xkb layout.'
complete -c wxkb_switch -s l -l list    -d 'List avalible layous.'
complete -c wxkb_switch -s v -l version -d 'Print program version.'
complete -c wxkb_switch -s d -l debug   -d 'Enable debug mode.'