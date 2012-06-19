#include "common.hpp"

#include "format_sup.hpp"

#include <map>
#include <utility>
#include <vector>

#include <cstring>

static bool read_segments(std::istream* in, Subtitle& subtitle);

bool load_sup(std::istream* in, std::list<Subtitle> subs)
{
    unsigned int count = 0;
    while (!in->eof() && !in->fail())
    {
        Subtitle subtitle;
        if (!read_segments(in, subtitle))
        {
            return false;
        }
        subs.push_back(subtitle);
    }
    return count > 0 && !in->bad();
}

bool save_sup(std::ostream* out, std::list<Subtitle> subs)
{
    return false;
}

static inline u8 readu8(std::istream* in)
{
    u8 ret;
    in->read(reinterpret_cast<char*>(&ret), sizeof(ret));
    return ret;
}

static inline u16 readu16(std::istream* in)
{
    u16 ret;
    in->read(reinterpret_cast<char*>(&ret), sizeof(ret));
#ifdef WORDS_BIGENDIAN
    return ret;
#else
    return (ret >> 8) | ((ret & 0xff) << 8);
#endif
}

static inline u32 readu24(std::istream* in)
{
    u8 x = readu8(in);
    u16 y = readu16(in);
#ifdef WORDS_BIGENDIAN
    return (x << 16) | y;
#else
    return x | (y << 8);
#endif
}

static inline u32 readu32(std::istream* in)
{
    u32 ret;
    in->read(reinterpret_cast<char*>(&ret), sizeof(ret));
#ifdef WORDS_BIGENDIAN
    return ret;
#else
    return (ret >> 24) | ((ret & 0xff0000) >> 8) | ((ret & 0xff00) << 8) |
        ((ret & 0xff) << 24);
#endif
}

enum segment_type_t
{
    SEGMENT_TYPE_PALETTE = 0x14,
    SEGMENT_TYPE_IMAGE = 0x15,
    SEGMENT_TYPE_TIMECODES = 0x16,
    SEGMENT_TYPE_WINDOW = 0x17,
    SEGMENT_TYPE_END = 0x80,
};

class PaletteEntry
{
public:
    u8 index;
    u8 y, cr, cb, alpha;
};

#ifdef DEBUG_OUTPUT
static std::ostream& operator<<(std::ostream& out, const PaletteEntry& entry)
{
    return out << "[#" << (int)entry.index << " Y:" << (int)entry.y << " Cr:" << (int)entry.cr << " Cb:" << (int)entry.cb << " A:" << (int)entry.alpha << ']';
}
#endif

class Palette
{
public:
    u8 id, version;
    typedef std::list<PaletteEntry> entry_list;
    entry_list entries;
};

#ifdef DEBUG_OUTPUT
static std::ostream& operator<<(std::ostream& out, const Palette& pal)
{
    out << "{id:" << (int)pal.id << "/" << (int)pal.version << " entries:";
    // for (Palette::entry_list::const_iterator i(pal.entries.begin()); i != pal.entries.end(); i++)
    // {
    //     out << *i << ',';
    // }
    return out << '}';
}
#endif

enum image_flags_t
{
    IMAGE_FLAG_FIRST = 0x80,
    IMAGE_FLAG_LAST = 0x40
};

template<typename T>
class RefData
{
public:
    RefData(T* ptr)
    : ptr(ptr), ref(1)
    {
    }

    ~RefData()
    {
        assert(ref == 0);
        delete[] ptr;
    }

    void retain()
    {
        ref++;
    }

    void release()
    {
        assert(ref > 0);
        if (--ref == 0)
        {
            delete this;
        }
    }

    T* ptr;

private:
    unsigned int ref;
};

class Image
{
public:
    Image()
        : data(NULL)
    {
    }

    Image(const Image& img)
    : id(img.id), version(img.version), flags(img.flags), width(img.width),
      height(img.height), total(img.total), size(img.size), data(img.data)
    {
        if (data != NULL)
        {
            data->retain();
        }
    }

    ~Image()
    {
        if (data != NULL)
        {
            data->release();
        }
    }

    u16 id;
    u8 version;
    u8 flags;

    u16 width, height;
    u32 total;

    u16 size;
    RefData<u8>* data;
};

#ifdef DEBUG_OUTPUT
static std::ostream& operator<<(std::ostream& out, const Image& img)
{
    return out << "{id:" << img.id << "/" << (int)img.version << " first:" << ((img.flags & IMAGE_FLAG_FIRST) ? 'Y' : 'N') << " last:" << ((img.flags & IMAGE_FLAG_LAST) ? 'Y' : 'N') << " size:" << img.width << "x" << img.height << " total: " << img.total << " bytes:" << img.size << '}';
}
#endif

class Window
{
public:
    u8 id;
    u16 x, y;
    u16 width, height;
};

#ifdef DEBUG_OUTPUT
static std::ostream& operator<<(std::ostream& out, const Window& wnd)
{
    return out << "{id:" << (int)wnd.id << " pos:" << wnd.x << "x" << wnd.y << " size:" << wnd.width << "x" << wnd.height << '}';
}
#endif

enum object_flags_t
{
    OBJ_FLAG_CROPPED = 0x80,
    OBJ_FLAG_FORCED_ON = 0x40,
};

class Object
{
public:
    u16 id;
    u8 window_id;
    u8 flags;
    u16 x, y;
};

#ifdef DEBUG_OUTPUT
static std::ostream& operator<<(std::ostream& out, const Object& obj)
{
    return out << "{id:" << obj.id << " window:" << (int)obj.window_id << " cropped:" << ((obj.flags & OBJ_FLAG_CROPPED) ? 'Y' : 'N') << " forced:" << ((obj.flags & OBJ_FLAG_FORCED_ON) ? 'Y' : 'N') << " pos:" << obj.x << 'x' << obj.y << '}';
}
#endif

enum fps_t
{
    TIMECODE_FPS_UNKNOWN = 0xffff,
    TIMECODE_FPS_24 = 0x10,
};

enum timecode_palette_flags_t
{
    TIMECODE_PALETTE_FLAG_UPDATE = 0x80,
};

enum timecode_composition_state_t
{
    TIMECODE_COMP_STATE_EPOCH_START = 0x80,
    TIMECODE_COMP_STATE_NORMAL = 0x00,
};

class Timecode
{
public:
    u16 width, height;
    fps_t fps;
    u16 comp_num;
    u8 comp_state;
    u8 palette_flags;
    u8 palette_id;
    typedef std::list<Object> object_list;
    object_list objects;
};

#ifdef DEBUG_OUTPUT
static const char* fps2str(fps_t fps)
{
    switch (fps)
    {
    case TIMECODE_FPS_24:
        return "24";
    case TIMECODE_FPS_UNKNOWN:
        break;
    }
    return "unknown";
}
#endif

#ifdef DEBUG_OUTPUT
static std::ostream& operator<<(std::ostream& out, const Timecode& tc)
{
    out << "{size: " << tc.width << 'x' << tc.height << " fps:" << fps2str(tc.fps) << " component(num:" << tc.comp_num << " state:" << (int)tc.comp_state << ") palette_flags:" << (int)tc.palette_flags << " palette:" << (int)tc.palette_id << " objects:";
    for (Timecode::object_list::const_iterator i(tc.objects.begin()); i != tc.objects.end(); i++)
    {
        out << *i << ',';
    }
    return out << '}';
}
#endif

static bool read_palette(std::istream* in, Palette& palette, u16 length);
static bool read_image(std::istream* in, Image& image, u16 length);
static long read_window(std::istream* in, Window& window, u16 length);
static bool read_timecode(std::istream* in, Timecode& timecode, u16 length);
static long read_object(std::istream* in, Object& object, u16 length);

typedef std::vector<Palette> palette_list;
typedef std::map<u8, palette_list> palette_map;

typedef std::vector<Image> image_list;
typedef std::map<u16, image_list> image_map;

typedef std::vector<Window> window_list;

typedef std::list<Timecode> timecode_list;

struct entry
{
    palette_map palettes;
    image_map images;
    window_list windows;
    timecode_list timecodes;
};

static bool create_subimage(Subtitle& subtitle, entry& last, entry& current);

bool read_segments(std::istream* in, Subtitle& subtitle)
{
    unsigned int count = 0;
    entry last, current;
    for (;;)
    {
        char id[2];
        u32 presentation;
        u32 decoding;
        u8 type;
        u16 length;
        in->read(id, sizeof(id));
        if (in->eof() && in->gcount() == 0)
        {
            return count > 0;
        }
        presentation = readu32(in);
        decoding = readu32(in);
        type = readu8(in);
        length = readu16(in);
        if (in->fail() || id[0] != 'P' || id[1] != 'G')
        {
            std::cerr << "bad segment" << std::endl;
            return false;
        }
#ifdef DEBUG_OUTPUT
        std::cerr << "\tpts: " << presentation << " dts: " << decoding << std::endl;
#endif
        switch (type)
        {
        case SEGMENT_TYPE_PALETTE:
        {
            Palette palette;
            if (!read_palette(in, palette, length))
            {
                std::cerr << "bad palette" << std::endl;
                return false;
            }
#ifdef DEBUG_OUTPUT
            std::cerr << "palette: " << palette << std::endl;
#endif
            palette_map::iterator i = current.palettes.find(palette.id);
            if (i == current.palettes.end())
            {
                i = current.palettes.insert(std::make_pair(palette.id,
                                                           palette_list())).first;
            }
            i->second.push_back(palette);
            break;
        }
        case SEGMENT_TYPE_IMAGE:
        {
            Image image;
            if (!read_image(in, image, length))
            {
                std::cerr << "bad image" << std::endl;
                return false;
            }
#ifdef DEBUG_OUTPUT
            std::cerr << "image: " << image << std::endl;
#endif
            image_map::iterator i = current.images.find(image.id);
            if (i == current.images.end())
            {
                i = current.images.insert(std::make_pair(image.id, image_list())).first;
            }
            i->second.push_back(image);
            break;
        }
        case SEGMENT_TYPE_TIMECODES:
        {
            Timecode timecode;
            if (!read_timecode(in, timecode, length))
            {
                std::cerr << "bad timecode" << std::endl;
                return false;
            }
#ifdef DEBUG_OUTPUT
            std::cerr << "timecode: " << timecode << std::endl;
#endif
            current.timecodes.push_back(timecode);
            break;
        }
        case SEGMENT_TYPE_WINDOW:
        {
            u16 pos;
            u8 count;
            if (length < 1)
            {
                std::cerr << "bad window (1)" << std::endl;
                return false;
            }
            count = readu8(in);
            pos = 1;
            while (count-- > 0)
            {
                Window window;
                long ret = read_window(in, window, length - pos);
                if (ret < 0)
                {
                    std::cerr << "bad window (2)" << std::endl;
                    return false;
                }
#ifdef DEBUG_OUTPUT
                std::cerr << "window: " << window << std::endl;
#endif
                current.windows.push_back(window);
                pos += ret;
            }
            if (pos < length)
            {
                std::cerr << "bad window (3)" << std::endl;
                return false;
            }
            break;
        }
        case SEGMENT_TYPE_END:
            if (length != 0)
            {
                return false;
            }
#ifdef DEBUG_OUTPUT
            std::cerr << "end" << std::endl << std::endl;
#endif
            create_subimage(subtitle, last, current);
            break;
        default:
            std::cerr << "unknown: " << type << std::endl;
            in->ignore(length);
            break;
        }
    }
}

static void reset_entry(entry& entry)
{
    entry.timecodes.clear();
    entry.windows.clear();
    entry.palettes.clear();
    entry.images.clear();
}

static void transfer_entry(entry& dst, entry& src)
{
    reset_entry(dst);
    for (timecode_list::iterator i(src.timecodes.begin()); i != src.timecodes.end(); ++i)
    {
        dst.timecodes.push_back(*i);
    }
    for (window_list::iterator i(src.windows.begin()); i != src.windows.end(); ++i)
    {
        dst.windows.push_back(*i);
    }
    for (palette_map::iterator i(src.palettes.begin()); i != src.palettes.end(); ++i)
    {
        dst.palettes.insert(*i);
    }
    for (image_map::iterator i(src.images.begin()); i != src.images.end(); ++i)
    {
        dst.images.insert(*i);
    }
    reset_entry(src);
}

static bool render(SubImage& subimg, palette_list palettes, image_map images)
{
    if (images.size() != 1)
    {
        std::cerr << "other than one image id per timecode is not supported" << std::endl;
        return false;
    }
    if (palettes.size() != 1)
    {
        std::cerr << "other than one palette per timecode is not supported" << std::endl;
        return false;
    }
    std::memset(subimg.rgba, 0, subimg.width * subimg.height * sizeof(u32));
    image_list imgs = images[0];
    Image first = imgs.front();
    Image last = imgs.back();
    if ((first.flags & IMAGE_FLAG_FIRST) == 0 ||
        (last.flags & IMAGE_FLAG_LAST) == 0)
    {
        std::cerr << "invalid image sequence" << std::endl;
        return false;
    }
    u8 extended = 0;
    u8 arg1 = 0, arg2 = 0;
    u32* row = subimg.rgba;
    u32* pixel = row;
    u32 palette[256];
    u16 size;
    for (Palette::entry_list::iterator entry(palettes.front().entries.begin());
         entry != palettes.front().entries.end(); entry++)
    {
        u8 r, g, b, a;
        const float y = 298.082 * entry->y;
        r = (u16)(y +                       408.583 * entry->cr - 57067.776) >> 8;
        g = (u16)(y - 100.291 * entry->cb - 208.120 * entry->cr + 34707.456) >> 8;
        b = (u16)(y + 516.412 * entry->cb                       - 70870.016) >> 8;
        a = entry->alpha;
        palette[entry->index] = (r << 24) | (g << 16) | (b << 8) | a;
    }
    for (image_list::iterator img(imgs.begin()); img != imgs.end(); ++img)
    {
        u8* ptr = img->data->ptr;
        for (u16 i = 0; i < img->size; i++, ptr++)
        {
            switch (extended)
            {
            case 0:
                if (*ptr == 0)
                {
                    /* 00 ... */
                    extended = 1;
                }
                else
                {
                    /* Standard pixel */
                    *pixel++ = palette[*ptr];
                }
                break;
            case 1:
                if (*ptr == 0)
                {
                    /* 00 00 -> new line */
                    row += subimg.width;
                    pixel = row;
                    extended = 0;
                }
                else if ((*ptr & 0xc0) == 0x00)
                {
                    /* 00 0x -> x zeroes */
                    size = *ptr;
                    while (size-- > 0)
                    {
                        *pixel++ = palette[0];
                    }
                    extended = 0;
                }
                else
                {
                    extended = 2;
                    arg1 = *ptr;
                }
                break;
            case 2:
                switch (arg1 & 0xc0)
                {
                case 0x40:
                    /* 00 4x yy -> xyy zeroes */
                    size = ((arg1 & 0x3f) << 8) + *ptr;
                    while (size-- > 0)
                    {
                        *pixel++ = palette[0];
                    }
                    extended = 0;
                    break;
                case 0x80:
                    /* 00 8x yy -> x times value yy */
                    size = arg1 & 0x3f;
                    while (size-- > 0)
                    {
                        *pixel++ = palette[*ptr];
                    }
                    extended = 0;
                    break;
                case 0xc0:
                    arg2 = *ptr;
                    extended = 3;
                    break;
                default:
                    std::cerr << "invalid rle code" << std::endl;
                    return false;
                }
                break;
            case 3:
                /* 00 cx yy zz -> xyy times value zz */
                size = ((arg1 & 0x3f) << 8) | arg2;
                while (size-- > 0)
                {
                    *pixel++ = palette[*ptr];
                }
                extended = 0;
                break;
            }
        }
    }
    return true;
}

bool create_subimage(Subtitle& subtitle, entry& last, entry& current)
{
    if (current.timecodes.empty())
    {
        std::cerr << "entry without timecodes" << std::endl;
        reset_entry(last);
        reset_entry(current);
        return false;
    }
    if (current.timecodes.size() > 1)
    {
        std::cerr << "multiple timecodes before end?" << std::endl;
        reset_entry(last);
        reset_entry(current);
        return false;
    }
    Timecode current_tc = current.timecodes.front();
    if ((current_tc.comp_state & TIMECODE_COMP_STATE_EPOCH_START) != 0)
    {
        if (!last.timecodes.empty())
        {
            std::cerr << "both last and current are start timecodes" << std::endl;
            reset_entry(last);
        }
        transfer_entry(last, current);
        return true;
    }
    else
    {
        if (last.timecodes.empty())
        {
            std::cerr << "two non-start timecodes after each other" << std::endl;
            reset_entry(last);
            reset_entry(current);
            return false;
        }
    }

    Timecode last_tc = last.timecodes.front();

    if (subtitle.width == 0)
    {
        subtitle.width = last_tc.width;
        subtitle.height = last_tc.height;
    }
    if (subtitle.fps == 0 && last_tc.fps != TIMECODE_FPS_UNKNOWN)
    {
        switch (last_tc.fps)
        {
        case TIMECODE_FPS_24:
            subtitle.fps = 24;
            break;
        case TIMECODE_FPS_UNKNOWN:
            break;
        }
    }

    for (Timecode::object_list::iterator i(last_tc.objects.begin());
         i != last_tc.objects.end(); ++i)
    {
        Window wnd = last.windows[i->window_id];
        SubImage subimg(wnd.width, wnd.height);
        subimg.x = wnd.x;
        subimg.y = wnd.y;
        subimg.forced = (i->flags & OBJ_FLAG_FORCED_ON);

        render(subimg, last.palettes[last_tc.palette_id], last.images);

        subtitle.images.push_back(subimg);
    }

    reset_entry(last);
    reset_entry(current);
    return true;
}

bool read_palette(std::istream* in, Palette& palette, u16 length)
{
    if (length < 2)
    {
        return false;
    }
    palette.id = readu8(in);
    palette.version = readu8(in);
    length -= 2;
    if ((length % 5) != 0)
    {
        return false;
    }
    while (length > 0)
    {
        PaletteEntry entry;
        entry.index = readu8(in);
        entry.y = readu8(in);
        entry.cr = readu8(in);
        entry.cb = readu8(in);
        entry.alpha = readu8(in);
        palette.entries.push_back(entry);
        length -= 5;
    }
    return true;
}

bool read_image(std::istream* in, Image& image, u16 length)
{
    if (length < 4)
    {
        return false;
    }
    image.id = readu16(in);
    image.version = readu8(in);
    image.flags = readu8(in);
    length -= 4;
    if ((image.flags & IMAGE_FLAG_FIRST) == IMAGE_FLAG_FIRST)
    {
        if (length < 7)
        {
            return false;
        }
        image.total = readu24(in);
        image.width = readu16(in);
        image.height = readu16(in);
        length -= 7;
    }
    image.size = length;
    image.data = new RefData<u8>(new u8[image.size]);
    in->read(reinterpret_cast<char*>(image.data->ptr), image.size);
    return true;
}

long read_window(std::istream* in, Window& window, u16 length)
{
    if (length < 9)
    {
        return -1;
    }
    window.id = readu8(in);
    window.x = readu16(in);
    window.y = readu16(in);
    window.width = readu16(in);
    window.height = readu16(in);
    return 9;
}

bool read_timecode(std::istream* in, Timecode& timecode, u16 length)
{
    u8 count;
    u16 pos;
    if (length < 11)
    {
        return false;
    }
    timecode.width = readu16(in);
    timecode.height = readu16(in);
    switch (readu8(in))
    {
    case TIMECODE_FPS_24:
        timecode.fps = TIMECODE_FPS_24;
        break;
    default:
        timecode.fps = TIMECODE_FPS_UNKNOWN;
        break;
    }
    timecode.comp_num = readu16(in);
    timecode.comp_state = readu8(in);
    timecode.palette_flags = readu8(in);
    timecode.palette_id = readu8(in);
    count = readu8(in);
    pos = 11;
    while (count-- > 0)
    {
        Object object;
        long ret = read_object(in, object, length - pos);
        if (ret < 0)
        {
            return false;
        }
        pos += ret;
        timecode.objects.push_back(object);
    }
    return pos == length;
}

long read_object(std::istream* in, Object& object, u16 length)
{
    if (length < 8)
    {
        return -1;
    }
    object.id = readu16(in);
    object.window_id = readu8(in);
    object.flags = readu8(in);
    object.x = readu16(in);
    object.y = readu16(in);
    return 8;
}
