<?php

class Smasher
{

    protected $config_xml;
    protected $temp_dir = '/tmp';
    protected $root_dir = NULL;

    protected function load_config($config_file)
    {
        $this->config_xml = @simplexml_load_file($config_file);
        if (!$this->config_xml) {
            throw new Exception('Cannot load config file: ' . $config_file);
        }
    }

    protected function get_group_xml($id)
    {
        $group_xml = $this->config_xml->xpath("group[@id='$id']");
        return count($group_xml) === 1 ? $group_xml[0] : NULL;
    }

    protected function concatenate($files)
    {
        $temp_name = tempnam($this->temp_dir, '.smasher-');
        $temp_file = fopen($temp_name, 'w+');

        foreach ($files as $file) {
            fwrite($temp_file, file_get_contents($file));
            fwrite($temp_file, "\n");
        }

        fclose($temp_file);
        return $temp_name;
    }

    public function build($group, $type)
    {
        $group_xml = $this->get_group_xml($group);
        if (!$group_xml) {
            throw new Exception('Invalid group: ' . $group);
        }

        $files = array();
        foreach ($group_xml->children() as $item){
            $itemType = $item->getName();

            if($itemType === 'file'){
                $file = (string) $item['src'];
                if(preg_match("/\." . $type . "$/i", $file)){
                    $files []= $this->root_dir . $file;
                }
            }
            else if($itemType === 'dir'){
                $dirFiles = scandir($item['src']);
                foreach($dirFiles as $dirFile)
                {
                    if(preg_match("/\." . $type . "$/i", $dirFile))
                    {
                        $files []= $this->root_dir . $item['src'] . $dirFile;
                    }
                }
            }
        }

        $concatenated_file = $this->concatenate($files);

        $result = file_get_contents($concatenated_file);

        unlink($concatenated_file);
        return $result;
    }

    public function __construct($config_file)
    {
        $this->load_config($config_file);
    }
}