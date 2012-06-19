#include "common.hpp"

#include "format_sup.hpp"

#include <map>
#include <utility>

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

typedef std::list<Palette> palette_list;
typedef std::map<u8, palette_list> palette_map;

typedef std::list<Image> image_list;
typedef std::map<u16, image_list> image_map;

typedef std::list<Window> window_list;

bool read_segments(std::istream* in, Subtitle& subtitle)
{
    unsigned int count = 0;
    palette_map palettes;
    image_map images;
    window_list windows;
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
            palette_map::iterator i = palettes.find(palette.id);
            if (i == palettes.end())
            {
                i = palettes.insert(std::make_pair(palette.id,
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
            image_map::iterator i = images.find(image.id);
            if (i == images.end())
            {
                i = images.insert(std::make_pair(image.id, image_list())).first;
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
                windows.push_back(window);
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
            break;
        default:
            std::cerr << "unknown: " << type << std::endl;
            in->ignore(length);
            break;
        }
    }
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
