<?xml version="1.0" encoding="UTF-8"?>
<!-- ****************************************************************************** -->
<!--                                                                                -->
<!--  This XSLT stylesheet, based on S4paGran2HTML.xsl v 1.9 2008/09/30, transforms -->
<!--  Granule level SDP TOOLKIT produced Meta-data files for display (2010/05/03)   -->
<!--                                                                                -->
<!-- ****************************************************************************** -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:output method="html" encoding="UTF-8" indent="yes"/>
  
  <!-- Exclude DTDVersion and PGEVersionClass elements from schema if present  -->
  <!-- PGEVersion will be included in the Granule section of the displayed.    -->
  <xsl:template match="DTDVersion|PGEVersionClass"/>
  
  
  <xsl:template match="/">
    <html xmlns="http://www.w3.org/1999/xhtml">
      <head><title>Granule Metadata</title></head>
      <body>
        <xsl:apply-templates/>
      </body>
    </html>
  </xsl:template>
  
  <xsl:template match="DataCenterId">
    <h3 style="margin-bottom: 0;"><strong>Data Center</strong></h3>
    <blockquote style="margin-top: 0;">
      <table width="700" border="1">
        <tr>
          <td width="200"><strong>DataCenterId</strong></td>
          <td><xsl:value-of select="/GranuleMetaDataFile/DataCenterId"/></td>
        </tr>
      </table>
    </blockquote>    
  </xsl:template>

  <xsl:template match="CollectionMetaData">
    <h3 style="margin-bottom: 0;"><strong>Collection</strong></h3>
    <blockquote style="margin-top: 0;">
      <table width="700" border="1">
        <tr>
          <td width="200"><strong>ShortName</strong></td>
          <td><xsl:value-of select="ShortName"/></td>
        </tr>
        <xsl:if test="LongName">
          <tr>
            <td width="200"><strong>LongName</strong></td>
            <td><xsl:value-of select="LongName"/></td>
          </tr>
        </xsl:if>
        <tr>
          <td width="200"><strong>VersionID</strong></td>
          <td><xsl:value-of select="VersionID"/></td>
        </tr>
        <xsl:if test="URL">
          <tr>
            <td width="200"><strong>URL</strong></td>
            <td><xsl:element name="a"><xsl:attribute name="target">new</xsl:attribute><xsl:attribute name="href"><xsl:for-each select="URL/@*"><xsl:if test="name() = 'xlink:href'"><xsl:value-of select="."/></xsl:if></xsl:for-each></xsl:attribute><xsl:for-each select="URL/@*"><xsl:if test="name() = 'xlink:title'"><xsl:value-of select="."/></xsl:if></xsl:for-each></xsl:element></td>
          </tr>
        </xsl:if>
      </table>
    </blockquote>    
  </xsl:template>
  
  <xsl:template match="ECSDataGranule">
    <h3 style="margin-bottom: 0;"><strong>Granule</strong></h3>
    <blockquote style="margin-top: 0;">
      <table width="700" border="1">
        
        <xsl:if test="ReprocessingPlanned">
          <tr>
            <td><strong>ReprocessingPlanned</strong></td>
            <td colspan="2"><xsl:value-of select="ReprocessingPlanned"/></td>
          </tr>
        </xsl:if>
        <tr>
          <td width="200"><strong>LocalGranuleID</strong></td>
          <td width="700" colspan="2"><xsl:value-of select="LocalGranuleID"/></td>
        </tr>
        <xsl:if test="Format">
          <tr>
            <td width="200"><p><strong>Format</strong> </p></td>
            <td width="700" colspan="2"><xsl:value-of select="Format"/></td>
          </tr>
        </xsl:if>
        
        <xsl:if test="SizeBytesDataGranule">
          <tr>
            <td width="200"><p><strong>Total Size</strong> </p></td>
            <td width="700" colspan="2"><xsl:value-of select="ECSDataGranule/SizeBytesDataGranule"/> Bytes</td>
          </tr>
        </xsl:if>
        
        <xsl:if test="InsertDateTime">
          <tr>
            <td width="200"><strong>Insert Time</strong> </td>
            <td width="700" colspan="2"><xsl:value-of select="ECSDataGranule/InsertDateTime"/></td>
          </tr>
        </xsl:if>
        
        <xsl:if test="../PGEVersionClass">
          <tr>
            <td width="200"><strong>PGE Version</strong></td>
            <td width="700" colspan="2"><xsl:value-of select="../PGEVersionClass/PGEVersion"/></td>
          </tr>
        </xsl:if>
        <xsl:if test="DayNightFlag">
          <tr>
            <td width="200"><strong>Day/Night Flag</strong></td>
            <td width="700" colspan="2"><xsl:value-of select="DayNightFlag"/></td>
          </tr>
        </xsl:if>
        <xsl:if test="BrowseFile">
          <tr>
            <td width="200"><strong>Browse File</strong></td>
            <td width="700" colspan="2"><xsl:value-of select="BrowseFile"/></td>
          </tr>
        </xsl:if>
        <xsl:if test="LocalVersionID">
          <tr>
            <td width="200"><strong>LocalVersionID</strong></td>
            <td width="700" colspan="2"><xsl:value-of select="LocalVersionID"/></td>
          </tr>
        </xsl:if>
        <xsl:if test="ProductionDateTime">
          <tr>
            <td width="200"><strong>Production Date/Time</strong></td>
            <td  width="700" colspan="2"><xsl:value-of select="ProductionDateTime"/></td>
          </tr>
        </xsl:if>
        <tr>
          <td rowspan="2"><strong>Time Coverage</strong></td>
          <th>Begin Date Time</th>
          <th>End Date Time</th>
        </tr>
        <tr>
          <td><xsl:value-of select="../RangeDateTime/RangeBeginningDate"/>T<xsl:value-of select="../RangeDateTime/RangeBeginningTime"/>Z</td>
          <td><xsl:value-of select="../RangeDateTime/RangeEndingDate"/>T<xsl:value-of select="../RangeDateTime/RangeEndingTime"/>Z</td>
        </tr>
      </table>
    </blockquote> 
    <xsl:choose>
      <xsl:when test="Granulits">
        <xsl:apply-templates select="Granulits"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:apply-templates select="GranuleID"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="Granulits">
    <h3 style="margin-bottom: 0;"><strong>Files</strong></h3>
    <blockquote style="margin-top: 0;">
      <table width="700" border="1">
        <xsl:for-each select="Granulit">
          <tr>
            <td colspan="3"><strong><xsl:value-of select="FileName"/></strong></td>
          </tr>
          <tr>
            <td rowspan="2" width="200">Checksum</td>
            <td>Type</td>
            <td><xsl:value-of select="CheckSum/CheckSumType"/></td>
          </tr>
          <tr>
            <td>Value</td>
            <td><xsl:value-of select="CheckSum/CheckSumValue"/></td>
          </tr>
          <tr>
            <td>Size in Bytes</td>
            <td colspan="2"><xsl:value-of select="FileSize"/></td>
          </tr>
          <tr>
            <td>GranulitID</td>
            <td colspan="2"><xsl:value-of select="GranulitID"/></td>
          </tr>
        </xsl:for-each>
      </table>
    </blockquote>    
  </xsl:template>
  
  <xsl:template match="GranuleID">
    <h3 style="margin-bottom: 0;"><strong>Files</strong></h3>
    <blockquote style="margin-top: 0;">
      <table width="700" border="1">
        <tr>
          <td colspan="3"><strong><xsl:value-of select="."/></strong></td>
        </tr>
        <tr>
          <td rowspan="2" width="200">Checksum</td>
          <td>Type</td>
          <td><xsl:value-of select="../CheckSum/CheckSumType"/></td>
        </tr>
        <tr>
          <td>Value</td>
          <td><xsl:value-of select="../CheckSum/CheckSumValue"/></td>
        </tr>
        <tr>
          <td>Size in Bytes</td>
          <td colspan="2"><xsl:value-of select="../SizeBytesDataGranule"/></td>
        </tr>
      </table>
    </blockquote>    
  </xsl:template>
  
  <xsl:template match="RangeDateTime"/>
  
  <xsl:template match="PSAs">
    <h3 style="margin-bottom: 0;"><strong>Product Specific Attributes</strong></h3>
    <blockquote style="margin-top: 0;">
      <table width="700" border="1">
        <tr>
          <th width="200">Name</th>
          <th width="500">Value</th>
        </tr>
        <xsl:for-each select="PSA">
          <tr>
            <td><xsl:value-of select="PSAName"/></td>
            <td><xsl:value-of select="PSAValue"/></td>
          </tr>
        </xsl:for-each>
      </table>
    </blockquote>    
  </xsl:template>
  
  <xsl:template match="Platform">
    <h3 style="margin-bottom: 0;">
      <strong>Platform</strong></h3>
      <blockquote style="margin-top: 0;">
        <table width="700" border="1">
          <tr> <th width="200">Platform</th>
          <th width="200">Instrument</th>
          <th width="200">Sensor</th>
        </tr>
        <xsl:for-each select="Instrument">
          <xsl:for-each select="Sensor">
            <tr>
              <td><xsl:value-of select="../../PlatformShortName"/></td>
              <td><xsl:value-of select="../InstrumentShortName"/></td>
              <td><xsl:value-of select="SensorShortName"/></td>
            </tr>
          </xsl:for-each>
        </xsl:for-each>
      </table>
    </blockquote>    
  </xsl:template>
  
  <xsl:template match="MeasuredParameter">
    <h3 style="margin-bottom: 0;"><strong>Measured Parameters</strong></h3>
    <blockquote style="margin-top: 0;">
      <table width="700" border="1">
        <xsl:for-each select="MeasuredParameterContainer">
          <xsl:for-each select="*">
            <tr>
              <xsl:choose>
                <xsl:when test="name() = 'ParameterName'">
                  <td colspan="2"><strong>Parameter: </strong> <xsl:value-of select="text()"/></td>
                </xsl:when>
                <xsl:otherwise>
                  <td width="300"><strong><xsl:value-of select="name()"/>:</strong></td>
                </xsl:otherwise>
              </xsl:choose>
            </tr>
            
            <xsl:for-each select="*">
              <tr>
                <xsl:choose>
                  <xsl:when test="name() = 'AutomaticQualityFlag>Passed'">
                    <td colspan="2"><strong>Parameter:</strong> <xsl:value-of select="text()"/></td>
                  </xsl:when>
                  <xsl:otherwise>
                    <td width="300"><xsl:value-of select="name()"/></td>
                    <td width="300"><xsl:value-of select="text()"/></td>
                  </xsl:otherwise>
                </xsl:choose>
              </tr>
            </xsl:for-each>
          </xsl:for-each>
        </xsl:for-each>
      </table>
    </blockquote>
  </xsl:template>  
  
  <xsl:template match="SpatialDomainContainer">
    <xsl:choose>
      <xsl:when test="HorizontalSpatialDomainContainer/BoundingRectangle">
        <xsl:call-template name="SpatialDomainTemplate">
          <xsl:with-param name="colCount" select="5"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="HorizontalSpatialDomainContainer/GPolygon">
        <xsl:call-template name="SpatialDomainTemplate">
          <xsl:with-param name="colCount" select="4"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:call-template name="SpatialDomainTemplate">
          <xsl:with-param name="colCount" select="2"/>
        </xsl:call-template>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  
  <xsl:template match="OrbitCalculatedSpatialDomain">
    <h3 style="margin-bottom: 0;"><strong>Orbital Parameters</strong></h3>
    <blockquote style="margin-top: 0;">
      <table width="700" border="1">
        <xsl:for-each select="OrbitCalculatedSpatialDomainContainer/*/*">
          <tr>
            <td width="300"><xsl:value-of select="name()"/></td>
            <td width="300"><xsl:value-of select="text()"/></td>
          </tr>
        </xsl:for-each>
        <xsl:for-each select="OrbitCalculatedSpatialDomainContainer/*">
          <tr>
            <xsl:choose>
              <xsl:when test="name() = 'EquatorCrossingLongitude'">
                <td width="300">EquatorCrossingLongitude</td>
                <td width="300"><xsl:value-of select="text()"/></td>
              </xsl:when>
              <xsl:when test="name() = 'EquatorCrossingDate'">
                <td width="300">EquatorCrossingDate</td> 
                <td width="300"><xsl:value-of select="text()"/></td>
              </xsl:when>
              <xsl:when test="name() = 'EquatorCrossingTime'">
                <td width="300">EquatorCrossingTime</td> 
                <td width="300"><xsl:value-of select="text()"/></td>
              </xsl:when>
            </xsl:choose>
          </tr>
        </xsl:for-each>
      </table>
    </blockquote>
  </xsl:template>
  
  <xsl:template name="SpatialDomainTemplate">
    <xsl:param name="colCount"/>
    <h3 style="margin-bottom: 0;"><strong>Spatial Attributes</strong></h3>
    <blockquote style="margin-top: 0;">
      <table width="700" border="1">
        <xsl:variable name="colSpan" select="$colCount - 1"/>
        <xsl:for-each select="GranuleLocality/LocalityValue">
          <tr>
            <th width="200">Locality Value</th>
            <xsl:element name="td">
              <xsl:attribute name="colspan">
                <xsl:copy-of select="$colSpan"/>
              </xsl:attribute>
              <xsl:value-of select="."/>
            </xsl:element>
          </tr>
        </xsl:for-each>
        <xsl:for-each select="HorizontalSpatialDomainContainer/ZoneIdentifierClass/ZoneIdentifier">
          <tr>
            <th width="200">Zone Identifier</th>
            <xsl:element name="td">
              <xsl:attribute name="colspan">
                <xsl:copy-of select="$colSpan"/>
              </xsl:attribute>
              <xsl:value-of select="."/>
            </xsl:element>
          </tr>
        </xsl:for-each>
        
        <xsl:if test="HorizontalSpatialDomainContainer">
          <tr>
            <xsl:element name="th">
              <xsl:attribute name="colspan">
                <xsl:copy-of select="$colCount"/>
              </xsl:attribute>
              Horizontal Spatial Attributes
            </xsl:element>
          </tr>
          <xsl:if test="HorizontalSpatialDomainContainer/BoundingRectangle">
            <tr>
              <th rowspan="2">Bounding Rectangle</th>
              <th>West</th>
              <th>North</th>
              <th>East</th>
              <th>South</th>
            </tr>
            <tr>
              <td><xsl:value-of select="HorizontalSpatialDomainContainer/BoundingRectangle/WestBoundingCoordinate"/></td>
              <td><xsl:value-of select="HorizontalSpatialDomainContainer/BoundingRectangle/NorthBoundingCoordinate"/></td>
              <td><xsl:value-of select="HorizontalSpatialDomainContainer/BoundingRectangle/EastBoundingCoordinate"/></td>
              <td><xsl:value-of select="HorizontalSpatialDomainContainer/BoundingRectangle/SouthBoundingCoordinate"/></td>
            </tr>
          </xsl:if>
          
          <xsl:for-each select="HorizontalSpatialDomainContainer/GPolygon">
            <xsl:variable name="rowspan" select="count(Boundary/Point)+1"/>
            <tr>
              <xsl:element name="th">
                <xsl:attribute name="rowspan">
                  <xsl:copy-of select="$rowspan"/>
                </xsl:attribute>
                GPolygon
              </xsl:element>
              <th>Exclusion Flag</th>
              <th>Latitude</th>
              <th>Longitude</th>
            </tr>
            <xsl:for-each select="Boundary/Point">
              <tr>
                <td><xsl:value-of select="ExclusionFlag"/></td>
                <td><xsl:value-of select="PointLatitude"/></td>
                <td><xsl:value-of select="PointLongitude"/></td>
              </tr>
            </xsl:for-each>
            <tr>
            </tr>
          </xsl:for-each>
        </xsl:if>
      </table>
      
      <table width="700" border="1">
        <xsl:if test="VerticalSpatialDomain/VerticalSpatialDomainContainer">
          <tr>
            <xsl:element name="th">
              <xsl:attribute name="colspan">
                <xsl:copy-of select="$colCount"/>
              </xsl:attribute>
              Vertical Spatial Attributes
            </xsl:element>
          </tr>
          <xsl:for-each select="VerticalSpatialDomain/VerticalSpatialDomainContainer">
            <tr>
              <th rowspan="2" width="200">Domain</th>
              <th>Type</th>
              <td><xsl:value-of select="VerticalSpatialDomainType"/></td>
            </tr>
            <tr>
              <th>Value</th>
              <td><xsl:value-of select="VerticalSpatialDomainValue"/></td>
            </tr>
          </xsl:for-each>
        </xsl:if>
      </table>
    </blockquote>    
  </xsl:template>
  
  <xsl:template match="BoundingRectangle">
  </xsl:template>
  
  <xsl:template match="InputGranule">
    <h3 style="margin-bottom: 0;"><strong>Input Granule</strong></h3>
    <blockquote style="margin-top: 0;">
      <table width="700" border="1">
        <tr>
          <td width="200"><strong>InputPointer</strong></td>
          <td width="700" colspan="2"><xsl:value-of select="InputPointer"/></td>
        </tr>
      </table>
    </blockquote>    
  </xsl:template>
  
  <xsl:template match="ProducersMetaData">
    <h3 style="margin-bottom: 0;"><strong>Data Producer's Metadata</strong></h3>
    <blockquote style="margin-top: 0;">
      <table width="700" border="1">
        <tr>
          <td><pre><xsl:value-of select="."/></pre></td>
        </tr>
      </table>
    </blockquote>
  </xsl:template> 
</xsl:stylesheet>



































































































































