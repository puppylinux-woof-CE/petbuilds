#!/bin/sh

# Make a tar file with pets listed on the command line
# and updated Packages-puppy-*-official files.


red=$(echo -en '\033[31m')
green=$(echo -en '\033[32m')
regular=$(echo -en '\033[39m')


# Check some requirements before starting.
if [ "${PWD##*/}" != "petbuilds-out" ]; then
  echo "mk-tar.sh must be run from the petbuilds-out directory."
  exit 1
fi

if [ ! -d "puppylinux" ]; then
  echo "mk-tar.sh needs USE_PUPPYLINUX_REPO_FORMAT=yes in build.conf"
  exit 1
fi

if [ "$1" = "" ]; then
  echo
  echo "You must specify the generic names of the pets you"
  echo "want to include in the tar file on the command line."
  exit 1
fi

case $1 in
  -h*|--h*|-u*|--u*)
    echo
    echo "Usage: mk-tar.sh <generic name> [<generic name>]..."
    echo
    echo "Specify the generic names of the pets you want to include."
    echo "_DEV _DOC and _NLS will be automatically included."
    exit
    ;;
  -v*|--v*)
    echo "mk-tar.sh version 0.0.1"
    exit
    ;;
  -*)
    echo "Error: $1 not supported."
    exit 1
    ;;
  *\.pet)
    echo "Please use generic names, not file names."
    exit 1
    ;;
esac



# Find names of pets requested to be included.
find_names="-iname '${1}*'"
exclude_patterns="'\||${1}|\||${1}_DEV|\||${1}_DOC|\||${1}_NLS|"
shift
if [ "$1" != "" ]; then
  for one_name in $@ ; do
    find_names="${find_names} -o -iname '${one_name}*'"
    exclude_patterns="${exclude_patterns}\||${one_name}|\||${one_name}_DEV|\||${one_name}_DOC|\||${one_name}_NLS|"
  done
fi
exclude_patterns="${exclude_patterns}\|'"
#echo "exclude_patterns = ${exclude_patterns}"
pet_names=$(eval find puppylinux ${find_names})
#echo "pet_names = ${pet_names}"



# Get Packages-puppy-*-official for repos involved.
repo_names=$(echo "${pet_names}" | cut -f 2 -d '/' | cut -f 2 -d '-' | uniq)
#echo "repo_names = ${repo_names}"
for one_repo_name in ${repo_names} ; do

  if [ -f "ibibloi_Packages-puppy-${one_repo_name}-official" ]; then
    echo "Do you want to download new"
    echo "ibibloi_Packages-puppy-${one_repo_name}-official? (y/N)"
    read -n 1 download
    echo
    if [ "${download}" = "y" ]; then
      rm "ibibloi_Packages-puppy-${one_repo_name}-official"
    fi
  fi

  [ ! -f "ibibloi_Packages-puppy-${one_repo_name}-official" ] && \
    wget -O "ibibloi_Packages-puppy-${one_repo_name}-official" \
    "http://distro.ibiblio.org/puppylinux/Packages-puppy-${one_repo_name}-official"

done



# Sort Packages-puppy-*-official and check for differences.
for one_repo_name in ${repo_names} ; do
  LANG=C sort --field-separator='|' --key=2,2 \
    "ibibloi_Packages-puppy-${one_repo_name}-official" > \
    "ibibloi_Packages-puppy-${one_repo_name}-official_sorted"

  if [ ! -f "puppylinux/Packages-puppy-${one_repo_name}-official" ]; then
    echo -n > "Packages-puppy-${one_repo_name}-official_temp"
    for one_pet in $(echo "${pet_names}" | \
      grep "puppylinux/pet_packages-${one_repo_name}") ; do

      one_pet_name=${one_pet##puppylinux/pet_packages-${one_repo_name}/}
      #echo "one_pet_name = ${one_pet_name}"
      tar -xf "${one_pet}" --to-stdout "./${one_pet_name%.pet}/pet.specs" >> \
        "Packages-puppy-${one_repo_name}-official_temp" 2> /dev/null
    done
    grep -v "${exclude_patterns}" \
      "ibibloi_Packages-puppy-${one_repo_name}-official" >> \
      "Packages-puppy-${one_repo_name}-official_temp"

    LANG=C sort --field-separator='|' --key=2,2 \
      "Packages-puppy-${one_repo_name}-official_temp" > \
      "Packages-puppy-${one_repo_name}-official_sorted"

    cp "Packages-puppy-${one_repo_name}-official_sorted" \
      "puppylinux/Packages-puppy-${one_repo_name}-official"

  else
    LANG=C sort --field-separator='|' --key=2,2 \
      "puppylinux/Packages-puppy-${one_repo_name}-official" > \
      "Packages-puppy-${one_repo_name}-official_sorted"
  fi

  repo_diff=$(diff "ibibloi_Packages-puppy-${one_repo_name}-official_sorted" \
    "Packages-puppy-${one_repo_name}-official_sorted")

  #echo "repo_diff = ${repo_diff}"
  if [ -n "${repo_diff}" ]; then
    changed_lines=$(echo "${repo_diff}" | grep '^<\|^>')
    #echo "changed_lines = ${changed_lines}"
    out_of_sync=$(echo "${changed_lines}" | grep -v ${exclude_patterns})
    #echo "out_of_sync = ${out_of_sync}"
    if [ -n "${out_of_sync}" ]; then
      extra_lines=$(echo "${out_of_sync}" | grep '^>')
      if [ -n "${extra_lines}" ]; then
        echo "Packages-puppy-${one_repo_name}-official appears to be out of sync."
        echo "These are the extra lines:"
        echo "${extra_lines}"
        echo
        echo "Please fix manually."
        exit 1
      fi
      missing_lines=$(echo "${out_of_sync}" | grep '^<')
      if [ -n "${missing_lines}" ]; then
        echo "The local copy of Packages-puppy-${one_repo_name}-official"
        echo "is missing lines that are in the copy from ibiblio.org"
        echo "Do you want to add the missing lines? (y/n)"
        read -n 1 add_lines
        echo
        if [ "${add_lines}" != "y" ]; then
          exit 1
        else
          mv "Packages-puppy-${one_repo_name}-official_sorted" \
            "Packages-puppy-${one_repo_name}-official_temp"

          echo "${missing_lines}" | sed -e 's/^< //g' >> \
            "Packages-puppy-${one_repo_name}-official_temp"

          LANG=C sort --field-separator='|' --key=2,2 \
            "Packages-puppy-${one_repo_name}-official_temp" > \
            "Packages-puppy-${one_repo_name}-official_sorted"

          cp "Packages-puppy-${one_repo_name}-official_sorted" \
            "puppylinux/Packages-puppy-${one_repo_name}-official"
        fi

      fi
    fi
    update_repos="${update_repos} ${one_repo_name}"
  else
    echo "Skipping ${one_repo_name}, ibiblio.org already up to date."
  fi

done



# Select pets for tar file and update Packages-puppy-*-official
for one_repo_name in ${update_repos} ; do

  repo_diff=$(diff "ibibloi_Packages-puppy-${one_repo_name}-official_sorted" \
    "Packages-puppy-${one_repo_name}-official_sorted")

  missing_lines=$(echo "${repo_diff}" | grep '^<')
  if [ -n "${missing_lines}" ]; then
    echo
    echo "These are the existing entries for the pets selected:"
    echo "${missing_lines}"
    echo
    echo "What do you want to do?"
    echo "1 keep existing (new entries will still be added)"
    echo "2 replace existing (default)"
    read -n 1 existing_choice
    echo
    case ${existing_choice} in
      1)
        mv "Packages-puppy-${one_repo_name}-official_sorted" \
          "Packages-puppy-${one_repo_name}-official_temp"

        echo "${missing_lines}" | sed -e 's/^< //g' >> \
          "Packages-puppy-${one_repo_name}-official_temp"

        LANG=C sort --field-separator='|' --key=2,2 \
          "Packages-puppy-${one_repo_name}-official_temp" > \
          "Packages-puppy-${one_repo_name}-official_sorted"

        cp "Packages-puppy-${one_repo_name}-official_sorted" \
          "puppylinux/Packages-puppy-${one_repo_name}-official"
        ;;
      *) : ;;
    esac
  fi

  extra_lines=$(echo "${repo_diff}" | grep '^>')
  while true ; do
    one_line=${extra_lines%%'
'*}
    extra_lines=${extra_lines#*'
'}
    [ "$extra_lines" = "$one_line" ] && extra_lines='' # EOF

    one_pet_name=$(echo "${one_line}" | cut -f 8 -d '|')
    #echo "one_pet_name = ${one_pet_name}"

    tar_files="${tar_files}
puppylinux/pet_packages-${one_repo_name}/${one_pet_name}"

    [ -z "$extra_lines" ] && break
  done
  tar_files="${tar_files}
puppylinux/Packages-puppy-${one_repo_name}-official
"

done
#echo "tar_files = ${tar_files}"



# Show changes to Packages-puppy-*-official for approval.
all_package_lists=$(echo "${tar_files}" | grep 'Packages-puppy-')
#echo "all_package_lists = ${all_package_lists}"
echo "These changes have been made to the package lists:"
for one_package_list in ${all_package_lists} ; do
  echo
  diff -U 0 "ibibloi_${one_package_list##puppylinux/}" "${one_package_list}" | \
    sed -e "s/^-/${red}-/" -e "s/^+/${green}+/" -e "s/$/${regular}/"
done
echo
echo "These files will be included in the tar file:"
echo "${tar_files}"
echo
echo "OK to proceed? (Y/n)"
read -n 1 proceed
echo
if [ "${proceed}" = "y" -o "${proceed}" = "" ]; then
  tar -cJf petbuilds-out_$(date +%F).tar.xz ${tar_files}
  if [ "$?" = "0" ]; then
    echo "Successfully created petbuilds-out_$(date +%F).tar.xz"
  else
    echo "Error creating petbuilds-out_$(date +%F).tar.xz"
    exit 1
  fi
fi
