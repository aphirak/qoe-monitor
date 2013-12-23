# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def configure(conf):
  #conf.env.append_value("CXXFLAGS", ["-ggdb", "-g3", "-O0", "-D__STDC_CONSTANT_MACROS"])
  #conf.env.append_value("CFLAGS", ["-ggdb", "-g3", "-O0", "-D__STDC_CONSTANT_MACROS"])
  #conf.env.append_value("LINKFLAGS", ["-ldl","-lavcodec", "-lavformat", "-lavutil"])
  #conf.check_cxx(lib='avutil')
  #conf.check_cxx(lib='avcodec')
  #conf.check_cxx(lib='avformat')
  #conf.pkg_check_modules('avformat', 'libavformat', mandatory=True)
  conf.env['libavformat']= conf.check(mandatory=True, lib='avformat', uselib_store='libavformat')
  conf.env['libavcodec']= conf.check(mandatory=True, lib='avcodec', uselib_store='libavcodec')
  conf.env['libavutil']= conf.check(mandatory=True, lib='avutil', uselib_store='libavutil')
  #conf.env['ldl']= conf.check(mandatory=True, lib='dl', uselib_store='LDL')



def build(bld):
    module = bld.create_ns3_module('qoe-monitor', ['core'])
    module.source = [
    	'model/container.cc',
        'model/format.cc',
        'model/fragmentation-unit-header.cc',
        'model/h264-packetizer.cc',
        'model/mpeg4-container.cc',
        'model/multimedia-application-receiver.cc',
        'model/multimedia-application-sender.cc',
        'model/multimedia-file-rebuilder.cc',
        'model/nal-unit-header.cc',
        'model/packetizer.cc',
        'model/pcm-mu-law-packetizer.cc',
        'model/pcm-noise-metric.cc',
        'model/psnr-metric.cc',
        'model/rtp-protocol.cc',
        'model/simulation-dataset.cc',
        'model/ssim-metric.cc', 
        'model/wav-container.cc',
        ]

    module.use.append("libavformat")
    module.use.append("libavcodec")
    module.use.append("libavutil")
    module.use.append("LDL")


    module_test = bld.create_ns3_module_test_library('qoe-monitor')
    module_test.source = []

#    headers = bld.new_task_gen(features=['ns3header'])
    headers = bld(features='ns3header')
    headers.module = 'qoe-monitor'
    headers.source = [
    	'model/container.h',
        'model/format.h',
        'model/fragmentation-unit-header.h',
        'model/h264-packetizer.h',
        'model/metric.h',
        'model/mpeg4-container.h',
        'model/multimedia-application-receiver.h',
        'model/multimedia-application-sender.h',
        'model/multimedia-file-rebuilder.h',
        'model/nal-unit-header.h',
        'model/packetizer.h',
        'model/packet-trace-structure.h',
        'model/pcm-mu-law-packetizer.h',
        'model/pcm-noise-metric.h',
        'model/psnr-metric.h',
        'model/rtp-protocol.h',
        'model/simulation-dataset.h',
        'model/ssim-metric.h', 
        'model/wav-container.h',
        ]


    if bld.env['ENABLE_EXAMPLES']:
        bld.recurse('examples')
