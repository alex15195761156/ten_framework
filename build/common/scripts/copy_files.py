#
# This file is part of the TEN Framework project.
# See https://github.com/TEN-framework/ten_framework/LICENSE for license
# information.
#
import sys
import shutil
import os
from build.scripts import fs_utils


def main(src_files: list[str], dst_files: list[str]):
    for src_file, dst_file in zip(src_files, dst_files):
        dst_dir = os.path.dirname(dst_file)

        # Create the destination folder if not exists.
        if not os.path.exists(dst_dir):
            os.mkdir(dst_dir)

        if os.path.isdir(src_file):
            fs_utils.remove_tree(dst_file)

            shutil.copytree(
                src_file,
                dst_file,
                False,
                ignore=shutil.ignore_patterns("**/tsconfig.tsbuildinfo"),
            )
        else:
            fs_utils.copy(src_file, dst_file)


if __name__ == "__main__":
    files = sys.argv[1:]

    # Perform floor division.
    copy_size = len(files) // 2

    src_files = files[:copy_size]
    dst_files = files[copy_size:]

    main(src_files, dst_files)