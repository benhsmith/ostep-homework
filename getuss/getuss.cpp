#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>

#include <fcntl.h> /* open */
#include <stdint.h> /* uint64_t  */
#include <stdlib.h> /* size_t */
#include <unistd.h> /* pread, sysconf */

using namespace std;

typedef struct {
    uint64_t pfn : 54;
    unsigned int soft_dirty : 1;
    unsigned int file_page : 1;
    unsigned int swapped : 1;
    unsigned int present : 1;
} PagemapEntry;

bool pagemap_get_entry(PagemapEntry *entry, int pagemap_fd, uintptr_t vaddr)
{
	size_t nread;
	ssize_t ret;
	uint64_t data;

	nread = 0;
	while (nread < sizeof(data)) {
		ret = pread(pagemap_fd, &data, sizeof(data),
				(vaddr / sysconf(_SC_PAGE_SIZE)) * sizeof(data) + nread);
		nread += ret;
		if (ret <= 0) {
			return false;
		}
	}
	entry->pfn = data & (((uint64_t)1 << 54) - 1);
	entry->soft_dirty = (data >> 54) & 1;
	entry->file_page = (data >> 61) & 1;
	entry->swapped = (data >> 62) & 1;
	entry->present = (data >> 63) & 1;
	return true;
}

bool read_regions(pid_t pid)
{
    string maps_filename = "/proc/" + to_string(pid) + "/maps";
    string pagemaps_filename = "/proc/" + to_string(pid) + "/pagemap";
    string kpagecount_filename = "/proc/kpagecount";

    int pagemaps_fd = open(pagemaps_filename.c_str(), O_RDONLY);
    if (pagemaps_fd < 0)
    {
        cerr << "Failed to open " << pagemaps_filename << endl;
        return false;
    }

    int kpagecount_fd = open(kpagecount_filename.c_str(), O_RDONLY);
    if (pagemaps_fd < 0)
    {
        cerr << "Failed to open " << kpagecount_filename << endl;
        return false;
    }

    ifstream maps_file(maps_filename);

    string line;
    while (std::getline(maps_file, line))
    {
        size_t dash = line.find("-");
        if (dash == string::npos)
        {
            cerr << "Missing dash on line: " << line << endl;
            return false;
        }

        size_t space = line.find(" ", dash);
        if (space == string::npos)
        {
            cerr << "Missing space on line: " << line << endl;
            return false;
        }

        size_t start = stoul(line.substr(0, dash), nullptr, 16);
        size_t end = stoul(line.substr(dash + 1, space), nullptr, 16);

        cout << start << " - " << end << endl;
        PagemapEntry pentry;

        if (!pagemap_get_entry(&pentry, pagemaps_fd, start))
        {
            cerr << "Failed to get pagemap entry for " << start << endl;
        }

        cout << pentry.pfn << endl;

        uint64_t count;
        ssize_t ret;
        ret = pread(kpagecount_fd, &count, sizeof(count), pentry.pfn);
        if (ret != sizeof(count))
        {
            cerr << "Wrong read size " << ret << endl;
            //return false;
        }

        cout << count << endl;
    }

    return true;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 1;
    }

    pid_t pid = atoi(argv[1]);

    std::cout << "pid: " << pid << std::endl;

    if (!read_regions(pid))
    {
        return 2;
    }

    return 0;
}
