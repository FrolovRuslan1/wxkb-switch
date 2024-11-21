#!/bin/bash

# This is bash wxkb_switch completion

_wxkb_switch() {
    local cur prev opts

    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    opts="--help --version --debug --next --prev --list"
	
	

    case "${prev}" in
        -c|--config|-d|--device)
            COMPREPLY=($(compgen -f "${cur}"))
            return 0
            ;;
        *)
			COMPREPLY=()
			if [[ "$COMP_CWORD" -gt 1 ]]; then
				local tmparr=()

				for opt in ${opts}; do
					local isset=0

					for word in ${COMP_LINE}; do
						if [[ "${opt}" == "${word}" ]]; then
							isset=1
							break 
						fi
					done
					if [[ ${isset} -eq 0 ]]; then
						tmparr+=("${opt}")
					fi
				done

				value_list=$(printf "%s " "${tmparr[@]}")

				COMPREPLY=($(compgen -W "${value_list}" -- "${cur}"))
				
			else
				COMPREPLY=($(compgen -W "${opts}" -- "${cur}"))
			fi
			
            return 0
            ;;
    esac
}
complete -F _wxkb_switch wxkb_switch