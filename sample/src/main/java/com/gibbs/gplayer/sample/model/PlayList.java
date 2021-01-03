package com.gibbs.gplayer.sample.model;

import com.google.gson.Gson;

import java.util.List;

public class PlayList {
    public static PlayList createPlayList(String jsonString) {
        Gson gson = new Gson();
        return gson.fromJson(jsonString, PlayList.class);
    }

    private List<Links> links;

    public List<Links> getLinks() {
        return links;
    }

    public void setLinks(List<Links> links) {
        this.links = links;
    }

    public static class Links {
        /**
         * name : cctv1_2
         * url : http://cctvalih5ca.v.myalicdn.com/live/cctv1_2/index.m3u8
         */

        private String name;
        private String url;

        public String getName() {
            return name;
        }

        public void setName(String name) {
            this.name = name;
        }

        public String getUrl() {
            return url;
        }

        public void setUrl(String url) {
            this.url = url;
        }
    }
}
